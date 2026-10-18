#include "parser/parse_session.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

namespace beresta::detail
{
    // Сессия готовится к разбору: стеки пусты, DTD ещё не встречался
    // The session gets ready for parsing: the stacks are empty, no DTD has been met yet
    ParseSession::ParseSession(std::string_view text, const ParserOptions& options,
        ContentHandler& handler)
        : _inputs(text), _options(options), _handler(handler), _budget(options),
          _namespaces(), _dtd(), _validator(), _openElements(), _saxAttributes(), _attributeBuffer(),
          _pendingText(), _pendingSignificant(false), _textMainOffset(0), _tokenStart(0),
          _eventMainOffset(0), _rootSeen(false), _doctypeSeen(false), _standalone(false)
    {
    }

    /*
    ** Полный разбор. Ошибки модели (DomException из проверок и лимитов)
    ** переводятся в ParseException с позицией текущей конструкции; любая
    ** фатальная ошибка сначала сообщается ErrorHandler, затем бросается.
    **
    ** Full parsing. Model errors (DomException from checks and limits)
    ** are converted to ParseException with the position of the current construct; any
    ** fatal error is reported to the ErrorHandler first, then thrown.
    */
    void ParseSession::run()
    {
        _handler.setDocumentLocator(*this);
        _handler.startDocument();
        try
        {
            runLoop();
        }
        catch (const ParseException& problem)
        {
            reportFatal(problem);
        }
        catch (const DomException& error)
        {
            reportFatal(toParseException(error));
        }
        _handler.endDocument();
    }

    /*
    ** Диспетчер конструкций. Перед любой разметкой накопленный текст
    ** отдаётся обработчику. Сначала дешёвые проверки по первым символам
    ** (текст, "</", "<?", открывающий тег — самые частые случаи), и только
    ** внутри "<!" — длинные префиксы "<!--", "<![CDATA[", "<!DOCTYPE".
    **
    ** The construct dispatcher. Before any markup the accumulated text
    ** is given to the handler. First cheap checks by the first characters
    ** (text, "</", "<?", a start tag — the most frequent cases), and only
    ** inside "<!" — the long prefixes "<!--", "<![CDATA[", "<!DOCTYPE".
    */
    void ParseSession::parseNext()
    {
        XmlScanner& scanner = _inputs.current();

        if (scanner.peek() != '<')
        {
            parseText();
            return;
        }
        flushText();
        if (scanner.startsWith("</"))
            parseEndTag();
        else if (scanner.startsWith("<?"))
            parseProcessingInstruction();
        else if (!scanner.startsWith("<!"))
            parseStartTag();
        else if (scanner.startsWith("<!--"))
            parseComment();
        else if (scanner.startsWith("<![CDATA["))
            parseCData();
        else if (scanner.startsWith("<!DOCTYPE"))
            parseDoctype();
        else
            scanner.fail(i18n::format(i18n::Message::UNKNOWN_MARKUP));
    }

    /*
    ** Конец текущего источника. Исчерпанная сущность снимается со стека,
    ** но сначала проверяется, что она закрыла все открытые ею элементы.
    ** Возвращает true, когда закончился сам документ.
    **
    ** The end of the current source. An exhausted entity is popped from the stack,
    ** but first it is checked that it closed all elements it opened.
    ** Returns true when the document itself is over.
    */
    bool ParseSession::handleInputEnd()
    {
        while (_inputs.current().isAtEnd())
        {
            if (!_inputs.inEntity())
                return true;
            if (_openElements.size() != _inputs.top().openElements)
                _inputs.current().fail(i18n::format(i18n::Message::ENTITY_UNCLOSED_ELEMENT));
            _inputs.pop();
        }
        return false;
    }

    // Итоговые проверки документа
    // The final document checks
    void ParseSession::finish()
    {
        XmlScanner& main = _inputs.main();

        flushText();
        if (!_openElements.empty())
            main.fail(i18n::format(i18n::Message::TAG_NOT_CLOSED, {_openElements.back().name}));
        if (!_rootSeen)
            main.fail(i18n::format(i18n::Message::NO_ROOT_ELEMENT));
        _eventMainOffset = main.getOffset();
        if (_validator)
            _validator->finish();
    }
}
