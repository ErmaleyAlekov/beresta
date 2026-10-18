#include "parser/parse_session.hpp"
#include "parser/error_handler.hpp"

namespace beresta::detail
{
    // Строка и столбец текущего события вычисляются только по запросу
    // The line and column of the current event are computed only on request
    SourceLocation ParseSession::getLocation() const
    {
        ParsePosition position = _inputs.main().computePosition(_eventMainOffset);

        return SourceLocation{position.line, position.column};
    }

    // Метка — смещение текущего события в основном документе: строка и столбец не считаются
    // The mark is the offset of the current event in the main document: the line and column are not computed
    std::size_t ParseSession::getMark() const
    {
        return _eventMainOffset;
    }

    // Позиция по смещению; по возрастанию меток счёт идёт от прошлой позиции, без повторов
    // The position by an offset; in ascending order of marks counting goes on from the last position, without repetitions
    SourceLocation ParseSession::locate(std::size_t mark) const
    {
        ParsePosition position = _inputs.main().computePosition(mark);

        return SourceLocation{position.line, position.column};
    }

    /*
    ** Ошибка валидности восстановима: с ErrorHandler разбор продолжается
    ** (если обработчик не бросит сам), без него — прерывается сразу.
    **
    ** A validity error is recoverable: with an ErrorHandler parsing continues
    ** (unless the handler throws itself), without it parsing stops at once.
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
    // Remembers the position of the current construct for the Locator and the validator messages
    void ParseSession::markEvent() noexcept
    {
        _eventMainOffset = _inputs.mainOffsetFor(_tokenStart);
    }
}
