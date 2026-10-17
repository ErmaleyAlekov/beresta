#include "parser/parse_session.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::detail
{
    /*
    ** Текст читается кусками до '<' или '&'. Ссылки обрабатываются по ходу;
    ** если ссылка раскрыла сущность (на вершине стека — её текст), разбор
    ** возвращается в главный цикл. Текст копится в _pendingText и
    ** отдаётся обработчику целиком, когда начинается разметка: так текст,
    ** собранный из нескольких кусков и сущностей, приходит одним событием.
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
    ** Нормализация переводов строк по стандарту: \r\n и одиночный \r
    ** превращаются в \n. Заодно отмечается, есть ли непробельные символы.
    */
    void ParseSession::appendTextChunk(std::string_view chunk)
    {
        std::size_t position = 0;

        if (chunk.find("]]>") != std::string_view::npos)
            _inputs.current().fail("последовательность \"]]>\" недопустима в тексте");
        if (!_pendingSignificant && !utils::isWhitespaceOnly(chunk))
            _pendingSignificant = true;
        while (position < chunk.size())
        {
            std::size_t carriage = chunk.find('\r', position);
            if (carriage == std::string_view::npos)
            {
                _pendingText.append(chunk.substr(position));
                return;
            }
            _pendingText.append(chunk.substr(position, carriage - position));
            _pendingText += '\n';
            position = carriage + 1;
            if (position < chunk.size() && chunk[position] == '\n')
                ++position;
        }
    }

    /*
    ** Отдача накопленного текста. Вне корня допустимы только пробелы.
    ** Текст из одних пробелов — форматирование: он приходит как
    ** ignorableWhitespace и не попадает в DOM (если не просили сохранить).
    */
    void ParseSession::flushText()
    {
        if (_pendingText.empty())
            return;
        std::string text;
        text.swap(_pendingText);
        bool significant = _pendingSignificant;
        _pendingSignificant = false;
        if (_openElements.empty())
        {
            if (significant)
                _inputs.main().failAt("текст вне корневого элемента недопустим", _textMainOffset);
            return;
        }
        _eventMainOffset = _textMainOffset;
        if (_validator)
            _validator->text(significant);
        if (!significant && !_options.preserveWhitespaceText)
            _handler.ignorableWhitespace(text);
        else
            _handler.characters(text);
    }
}
