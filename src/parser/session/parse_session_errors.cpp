#include "parser/parse_session.hpp"
#include "i18n/i18n.hpp"
#include "parser/error_handler.hpp"

namespace beresta::detail
{
    // Декларация, затем конструкции до конца документа, затем итоговые проверки
    // The declaration, then the constructs to the end of the document, then the final checks
    void ParseSession::runLoop()
    {
        parseXmlDeclaration();
        while (!handleInputEnd())
        {
            _tokenStart = _inputs.current().getOffset();
            parseNext();
        }
        finish();
    }

    /*
    ** Фатальная ошибка: обработчик узнаёт о ней первым (например, чтобы
    ** записать в журнал), после чего разбор прерывается в любом случае —
    ** продолжать после нарушения синтаксиса XML стандарт запрещает.
    **
    ** A fatal error: the handler learns about it first (for example, to
    ** write it to a log), after which parsing stops in any case —
    ** the standard forbids continuing after an XML syntax violation.
    */
    void ParseSession::reportFatal(const ParseException& problem) const
    {
        if (_options.errorHandler != nullptr)
            _options.errorHandler->fatalError(problem);
        throw problem;
    }

    // Позиция — начало текущей конструкции (внутри сущности — место ссылки на неё)
    // The position is the start of the current construct (inside an entity — the place of its reference)
    ParseException ParseSession::toParseException(const DomException& error)
    {
        ParsePosition position = _inputs.main().computePosition(_inputs.mainOffsetFor(_tokenStart));
        std::string message = error.getMessage();

        if (_inputs.inEntity())
            message = i18n::format(i18n::Message::IN_ENTITY, {_inputs.top().entityName, message});
        return ParseException(message, position, error.getCode());
    }
}
