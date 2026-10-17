#include "parser/parse_session.hpp"
#include "parser/error_handler.hpp"

namespace beresta::detail
{
    // Строка и столбец текущего события вычисляются только по запросу
    SourceLocation ParseSession::getLocation() const
    {
        ParsePosition position = _inputs.main().computePosition(_eventMainOffset);

        return SourceLocation{position.line, position.column};
    }

    /*
    ** Ошибка валидности восстановима: с ErrorHandler разбор продолжается
    ** (если обработчик не бросит сам), без него — прерывается сразу.
    */
    void ParseSession::reportValidity(const std::string& message)
    {
        ParseException problem(message, _inputs.main().computePosition(_eventMainOffset),
            DomErrorCode::VALIDATION_ERROR);

        if (_options.errorHandler == nullptr)
            throw problem;
        _options.errorHandler->error(problem);
    }

    // Запоминаем позицию текущей конструкции для Locator и сообщений валидатора
    void ParseSession::markEvent() noexcept
    {
        _eventMainOffset = _inputs.mainOffsetFor(_tokenStart);
    }
}
