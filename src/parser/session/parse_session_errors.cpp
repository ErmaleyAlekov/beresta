#include "parser/parse_session.hpp"
#include "parser/error_handler.hpp"

namespace beresta::detail
{
    // Декларация, затем конструкции до конца документа, затем итоговые проверки
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
    */
    void ParseSession::reportFatal(const ParseException& problem) const
    {
        if (_options.errorHandler != nullptr)
            _options.errorHandler->fatalError(problem);
        throw problem;
    }

    // Позиция — начало текущей конструкции (внутри сущности — место ссылки на неё)
    ParseException ParseSession::toParseException(const DomException& error)
    {
        ParsePosition position = _inputs.main().computePosition(_inputs.mainOffsetFor(_tokenStart));
        std::string message = error.getMessage();

        if (_inputs.inEntity())
            message = "в сущности &" + std::string(_inputs.top().entityName) + ";: " + message;
        return ParseException(message, position, error.getCode());
    }
}
