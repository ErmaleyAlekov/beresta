#include "parser/parse_session.hpp"
#include "exception/dom_exception.hpp"

namespace beresta::detail
{
    // Сессия готовится к разбору: стеки пусты, DTD ещё не встречался
    ParseSession::ParseSession(std::string_view text, const ParserOptions& options,
        ContentHandler& handler)
        : _inputs(text), _options(options), _handler(handler), _budget(options),
          _namespaces(), _dtd(), _validator(), _openElements(), _saxAttributes(),
          _pendingText(), _pendingSignificant(false), _textMainOffset(0), _tokenStart(0),
          _eventMainOffset(0), _rootSeen(false), _doctypeSeen(false)
    {
    }

    /*
    ** Полный разбор. Ошибки модели (DomException из проверок и лимитов)
    ** переводятся в ParseException с позицией текущей конструкции; любая
    ** фатальная ошибка сначала сообщается ErrorHandler, затем бросается.
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
    ** отдаётся обработчику. Длинные префиксы ("<!--", "<![CDATA[",
    ** "<!DOCTYPE") проверяются раньше общего "<!".
    */
    void ParseSession::parseNext()
    {
        XmlScanner& scanner = _inputs.current();

        if (!scanner.startsWith("<"))
        {
            parseText();
            return;
        }
        flushText();
        if (scanner.startsWith("</"))
            parseEndTag();
        else if (scanner.startsWith("<!--"))
            parseComment();
        else if (scanner.startsWith("<![CDATA["))
            parseCData();
        else if (scanner.startsWith("<!DOCTYPE"))
            parseDoctype();
        else if (scanner.startsWith("<?"))
            parseProcessingInstruction();
        else if (scanner.startsWith("<!"))
            scanner.fail("неизвестная конструкция <!...>");
        else
            parseStartTag();
    }

    /*
    ** Конец текущего источника. Исчерпанная сущность снимается со стека,
    ** но сначала проверяется, что она закрыла все открытые ею элементы.
    ** Возвращает true, когда закончился сам документ.
    */
    bool ParseSession::handleInputEnd()
    {
        while (_inputs.current().isAtEnd())
        {
            if (!_inputs.inEntity())
                return true;
            if (_openElements.size() != _inputs.top().openElements)
                _inputs.current().fail("текст сущности содержит незакрытый элемент");
            _inputs.pop();
        }
        return false;
    }

    // Итоговые проверки документа
    void ParseSession::finish()
    {
        XmlScanner& main = _inputs.main();

        flushText();
        if (!_openElements.empty())
            main.fail("не закрыт тег <" + _openElements.back().name + ">");
        if (!_rootSeen)
            main.fail("документ не содержит корневого элемента");
        _eventMainOffset = main.getOffset();
        if (_validator)
            _validator->finish();
    }
}
