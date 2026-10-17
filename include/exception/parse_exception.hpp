#ifndef PARSE_EXCEPTION_HPP
#define PARSE_EXCEPTION_HPP

#include "exception/dom_exception.hpp"

#include <cstddef>
#include <string>

namespace beresta
{
    /*
    ** Позиция во входном тексте.
    ** offset — смещение в байтах от начала строки,
    ** line и column — человекочитаемые координаты (нумерация с единицы).
    ** Столбец считается в символах UTF-8, а не в байтах.
    */
    struct ParsePosition
    {
        std::size_t offset;
        std::size_t line;
        std::size_t column;
    };

    /*
    ** Исключение ошибки разбора или проверки документа.
    ** Дополняет базовое исключение точной позицией сбоя, чтобы
    ** пользователь сразу видел, в какой строке и столбце проблема.
    ** Код уточняет вид ошибки: PARSE_ERROR (синтаксис), NAMESPACE_ERROR,
    ** ENCODING_ERROR, VALIDATION_ERROR, RESOURCE_ERROR, LIMIT_EXCEEDED.
    ** Нулевая строка в позиции означает, что координаты неизвестны.
    */
    class ParseException : public DomException
    {
    public:
        ParseException(const std::string& message, ParsePosition position,
            DomErrorCode code = DomErrorCode::PARSE_ERROR);

        // Полная позиция ошибки
        const ParsePosition& getPosition() const noexcept;

        // Номер строки (с единицы)
        std::size_t getLine() const noexcept;

        // Номер столбца в символах UTF-8 (с единицы)
        std::size_t getColumn() const noexcept;

    private:
        // Собирает текст "строка N, столбец M: сообщение"
        static std::string buildMessage(const std::string& message,
            const ParsePosition& position);

        ParsePosition _position;
    };
}

#endif
