#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Конструктор исключения.
    ** Сразу собирает полный текст "[КОД] сообщение", чтобы what()
    ** был noexcept и не тратил ресурсы во время раскрутки стека.
    */
    DomException::DomException(DomErrorCode code, std::string message)
        : _code(code), _message(std::move(message))
    {
        _fullText = "[";
        _fullText += errorCodeToString(_code);
        _fullText += "] ";
        _fullText += _message;
    }

    // Возвращает категорию ошибки
    DomErrorCode DomException::getCode() const noexcept
    {
        return _code;
    }

    // Возвращает сообщение без префикса с кодом
    const std::string& DomException::getMessage() const noexcept
    {
        return _message;
    }

    // Возвращает полный текст ошибки для стандартного интерфейса std::exception
    const char* DomException::what() const noexcept
    {
        return _fullText.c_str();
    }

    /*
    ** Сопоставляет код ошибки с его строковым именем через таблицу,
    ** порядок которой совпадает с порядком перечисления DomErrorCode.
    ** static_assert ниже не даст забыть обновить таблицу при добавлении кода.
    */
    const char* errorCodeToString(DomErrorCode code) noexcept
    {
        static const char* const NAMES[] = {
            "INVALID_NAME", "INVALID_CONTENT", "HIERARCHY_VIOLATION",
            "INDEX_OUT_OF_RANGE", "NULL_NODE", "PARSE_ERROR", "NOT_FOUND",
            "NAMESPACE_ERROR", "IO_ERROR", "ENCODING_ERROR", "VALIDATION_ERROR",
            "XPATH_ERROR", "SCHEMA_ERROR", "RESOURCE_ERROR", "LIMIT_EXCEEDED"
        };
        static_assert(sizeof(NAMES) / sizeof(NAMES[0]) == BERESTA_ERROR_CODE_COUNT,
            "таблица имён должна совпадать с перечислением DomErrorCode");
        std::size_t index = static_cast<std::size_t>(code);

        if (index >= BERESTA_ERROR_CODE_COUNT)
            return "UNKNOWN";
        return NAMES[index];
    }
}
