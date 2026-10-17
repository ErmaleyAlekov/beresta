#include "capi/internal/capi_support.hpp"
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
    std::string_view requireText(const char* text, const char* what)
    {
        if (text == nullptr)
            throw InvalidArgumentException(std::string(what) + ": передан NULL");
        return std::string_view(text);
    }
}

// Версия библиотеки (статическая строка — освобождать не нужно)
extern "C" BERESTA_API const char* beresta_version(void)
{
    return BERESTA_VERSION_STRING;
}

/*
** Имя кода состояния. 1..15 — имена DomErrorCode из ядра (DRY: таблица
** не дублируется), остальные коды существуют только в Си-интерфейсе.
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
extern "C" BERESTA_API void beresta_string_free(char* text)
{
    std::free(text);
}
