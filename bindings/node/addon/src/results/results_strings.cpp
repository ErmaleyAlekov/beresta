#include "addon/results.hpp"
#include "addon/errors.hpp"
#include "capi/beresta_c.h"

namespace beresta::node
{
    // Строка UTF-8 -> строка JavaScript / A UTF-8 string -> a JavaScript string
    static napi_value createString(napi_env env, const char* text) noexcept
    {
        napi_value result = nullptr;

        napi().createStringUtf8(env, text, NAPI_AUTO_LENGTH, &result);
        return result;
    }

    /*
    ** Строка, принадлежащая вызывающему (char*): копируется в строку
    ** JavaScript и сразу освобождается через beresta_string_free — память
    ** библиотеки не переживает вызов, утечек нет.
    **
    ** A string owned by the caller (char*): copied into a JavaScript string
    ** and freed at once through beresta_string_free — the library memory
    ** does not outlive the call, there are no leaks.
    */
    napi_value makeOwnedString(napi_env env, char* text) noexcept
    {
        napi_value result = nullptr;

        if (text == nullptr)
        {
            if (beresta_last_error_code() != BERESTA_OK)
                return throwLastError(env);
            return makeNull(env);
        }
        result = createString(env, text);
        beresta_string_free(text);
        return result;
    }

    // Заимствованная строка (const char*): только копируется / A borrowed string (const char*): only copied
    napi_value makeBorrowedString(napi_env env, const char* text) noexcept
    {
        if (text == nullptr)
        {
            if (beresta_last_error_code() != BERESTA_OK)
                return throwLastError(env);
            return makeNull(env);
        }
        return createString(env, text);
    }
}
