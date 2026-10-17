#include "exception/parse_exception.hpp"

namespace beresta
{
    /*
    ** Конструктор: передаёт базовому классу код ошибки и
    ** сообщение, уже дополненное координатами ошибки.
    */
    ParseException::ParseException(const std::string& message, ParsePosition position,
        DomErrorCode code)
        : DomException(code, buildMessage(message, position)),
          _position(position)
    {
    }

    // Возвращает полную позицию ошибки
    const ParsePosition& ParseException::getPosition() const noexcept
    {
        return _position;
    }

    // Возвращает номер строки
    std::size_t ParseException::getLine() const noexcept
    {
        return _position.line;
    }

    // Возвращает номер столбца
    std::size_t ParseException::getColumn() const noexcept
    {
        return _position.column;
    }

    // Формирует сообщение вида "строка 3, столбец 7: текст ошибки"
    std::string ParseException::buildMessage(const std::string& message,
        const ParsePosition& position)
    {
        std::string text = "строка ";

        if (position.line == 0)
            return message;
        text += std::to_string(position.line);
        text += ", столбец ";
        text += std::to_string(position.column);
        text += ": ";
        text += message;
        return text;
    }
}
