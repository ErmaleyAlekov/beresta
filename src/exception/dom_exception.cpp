#include "exception/dom_exception.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Конструктор исключения.
    ** Сразу собирает полный текст "[КОД] сообщение", чтобы what()
    ** был noexcept и не тратил ресурсы во время раскрутки стека.
    **
    ** The exception constructor.
    ** It builds the full text "[CODE] message" at once so that what()
    ** is noexcept and spends no resources during stack unwinding.
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
    // Returns the error category
    DomErrorCode DomException::getCode() const noexcept
    {
        return _code;
    }

    // Возвращает сообщение без префикса с кодом
    // Returns the message without the code prefix
    const std::string& DomException::getMessage() const noexcept
    {
        return _message;
    }

    // Возвращает полный текст ошибки для стандартного интерфейса std::exception
    // Returns the full error text for the standard std::exception interface
    const char* DomException::what() const noexcept
    {
        return _fullText.c_str();
    }

    /*
    ** Сопоставляет код ошибки с его строковым именем через таблицу,
    ** порядок которой совпадает с порядком перечисления DomErrorCode.
    ** static_assert ниже не даст забыть обновить таблицу при добавлении кода.
    **
    ** Maps an error code to its string name through a table
    ** whose order matches the order of the DomErrorCode enumeration.
    ** The static_assert below does not let one forget to update the table when adding a code.
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
            "таблица имён должна совпадать с перечислением DomErrorCode / the name table must match DomErrorCode");
        std::size_t index = static_cast<std::size_t>(code);

        if (index >= BERESTA_ERROR_CODE_COUNT)
            return "UNKNOWN";
        return NAMES[index];
    }
}
