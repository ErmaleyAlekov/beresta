#include "exception/parse_exception.hpp"
#include "i18n/i18n.hpp"

namespace beresta
{
    /*
    ** Конструктор: передаёт базовому классу код ошибки и
    ** сообщение, уже дополненное координатами ошибки.
    **
    ** The constructor: passes the error code and the message, already
    ** extended with the error coordinates, to the base class.
    */
    ParseException::ParseException(const std::string& message, ParsePosition position,
        DomErrorCode code)
        : DomException(code, buildMessage(message, position)),
          _position(position)
    {
    }

    // Возвращает полную позицию ошибки
    // Returns the full error position
    const ParsePosition& ParseException::getPosition() const noexcept
    {
        return _position;
    }

    // Возвращает номер строки
    // Returns the line number
    std::size_t ParseException::getLine() const noexcept
    {
        return _position.line;
    }

    // Возвращает номер столбца
    // Returns the column number
    std::size_t ParseException::getColumn() const noexcept
    {
        return _position.column;
    }

    // Формирует сообщение вида "строка 3, столбец 7: текст ошибки"
    // Builds a message like "line 3, column 7: error text"
    std::string ParseException::buildMessage(const std::string& message,
        const ParsePosition& position)
    {
        if (position.line == 0)
            return message;
        return i18n::format(i18n::Message::AT_LINE_COLUMN, {std::to_string(position.line), std::to_string(position.column), message});
    }
}
