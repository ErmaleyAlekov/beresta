#include "parser/parse_session.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::detail
{
    /*
    ** Текст читается кусками до '<' или '&'. Ссылки обрабатываются по ходу;
    ** если ссылка раскрыла сущность (на вершине стека — её текст), разбор
    ** возвращается в главный цикл. Текст копится в _pendingText и
    ** отдаётся обработчику целиком, когда начинается разметка: так текст,
    ** собранный из нескольких кусков и сущностей, приходит одним событием.
    **
    ** Text is read in pieces up to '<' or '&'. References are processed along the way;
    ** if a reference expanded an entity (its text is on top of the stack), parsing
    ** returns to the main loop. The text accumulates in _pendingText and is
    ** given to the handler whole when markup starts: so text assembled
    ** from several pieces and entities arrives as one event.
    */
    void ParseSession::parseText()
    {
        XmlScanner& scanner = _inputs.current();

        if (_pendingText.empty())
            _textMainOffset = _inputs.mainOffsetFor(scanner.getOffset());
        while (true)
        {
            appendTextChunk(scanner.readUntilAny("<&"));
            if (scanner.peek() != '&')
                return;
            if (!handleTextReference())
                return;
        }
    }

    /*
    ** Кусок текста дописывается как есть: концы строк уже нормализованы
    ** при декодировании входа (decodeDocument), а символ \r, оставшийся
    ** в тексте сущности, пришёл из ссылки &#13; и обязан сохраниться.
    ** Заодно отмечается, есть ли непробельные символы.
    **
    ** A piece of text is appended as is: line endings are already normalized
    ** when the input is decoded (decodeDocument), and a \r character remaining
    ** in an entity text came from a &#13; reference and must be kept.
    ** Along the way it is noted whether there are non-whitespace characters.
    */
    void ParseSession::appendTextChunk(std::string_view chunk)
    {
        if (chunk.find("]]>") != std::string_view::npos)
            _inputs.current().fail(i18n::format(i18n::Message::CDATA_END_IN_TEXT));
        if (!_pendingSignificant && !utils::isWhitespaceOnly(chunk))
            _pendingSignificant = true;
        _pendingText.append(chunk);
    }

    /*
    ** Отдача накопленного текста. Вне корня допустимы только пробелы.
    ** Текст из одних пробелов — форматирование: он приходит как
    ** ignorableWhitespace и не попадает в DOM (если не просили сохранить).
    **
    ** Hands over the accumulated text. Outside the root only spaces are allowed.
    ** Whitespace-only text is formatting: it arrives as
    ** ignorableWhitespace and does not go into the DOM (unless keeping it was requested).
    */
    void ParseSession::flushText()
    {
        if (_pendingText.empty())
            return;
        bool significant = _pendingSignificant;
        _pendingSignificant = false;
        if (_openElements.empty())
        {
            if (significant)
                _inputs.main().failAt(i18n::format(i18n::Message::TEXT_OUTSIDE_ROOT), _textMainOffset);
            _pendingText.clear();
            return;
        }
        _eventMainOffset = _textMainOffset;
        if (_validator)
            _validator->text(significant);
        if (!significant && !_options.preserveWhitespaceText)
            _handler.ignorableWhitespace(_pendingText);
        else
            _handler.characters(_pendingText);
        _pendingText.clear();
    }
}
