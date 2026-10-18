#include "capi/internal/capi_support.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

#include <cstdlib>
#include <cstring>
#include <new>

namespace beresta::capi
{
    /*
    ** Строки для внешнего мира выделяются через malloc, а не new[]:
    ** освобождает их та же библиотека (beresta_string_free), поэтому
    ** выделение и освобождение всегда происходят в одной куче — это важно
    ** на Windows, где у DLL и приложения могут быть разные CRT.
    **
    ** Strings for the outside world are allocated with malloc, not new[]:
    ** the same library frees them (beresta_string_free), so allocation and
    ** freeing always happen in the same heap — this matters on Windows,
    ** where a DLL and an application may have different CRTs.
    */
    char* duplicateString(std::string_view text)
    {
        char* copy = static_cast<char*>(std::malloc(text.size() + 1));

        if (copy == nullptr)
            throw std::bad_alloc();
        if (!text.empty())
            std::memcpy(copy, text.data(), text.size());
        copy[text.size()] = '\0';
        return copy;
    }

    // Обязательная строка: NULL — ошибка аргумента с именем параметра
    // A mandatory string: NULL is an argument error with the parameter name
    std::string_view requireText(const char* text, const char* what)
    {
        if (text == nullptr)
            throw InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::ARGUMENT_NULL, {what}));
        return std::string_view(text);
    }
}

// Версия библиотеки (статическая строка — освобождать не нужно)
// The library version (a static string — no need to free it)
extern "C" BERESTA_API const char* beresta_version(void)
{
    return BERESTA_VERSION_STRING;
}

/*
** Имя кода состояния. 1..15 — имена DomErrorCode из ядра (DRY: таблица
** не дублируется), остальные коды существуют только в Си-интерфейсе.
**
** The status code name. 1..15 are the DomErrorCode names from the core (DRY: the table
** is not duplicated), the other codes exist only in the C interface.
*/
extern "C" BERESTA_API const char* beresta_status_name(int status)
{
    if (status == BERESTA_OK)
        return "OK";
    if (status >= 1 && status <= BERESTA_ERROR_CODE_COUNT)
        return beresta::errorCodeToString(static_cast<beresta::DomErrorCode>(status - 1));
    if (status == BERESTA_INVALID_ARGUMENT)
        return "INVALID_ARGUMENT";
    if (status == BERESTA_OUT_OF_MEMORY)
        return "OUT_OF_MEMORY";
    return "UNKNOWN_ERROR";
}

// Освобождение строки, выделенной duplicateString()
// Frees a string allocated by duplicateString()
extern "C" BERESTA_API void beresta_string_free(char* text)
{
    std::free(text);
}
