#include "addon/results.hpp"
#include "addon/errors.hpp"
#include "capi/beresta_c.h"

#include <cstdint>

namespace beresta::node
{
    // Дескриптор -> BigInt; NULL с ошибкой -> исключение, NULL без ошибки -> null
    // A handle -> BigInt; NULL with an error -> an exception, NULL without an error -> null
    napi_value makeHandle(napi_env env, void* handle) noexcept
    {
        napi_value result = nullptr;

        if (handle == nullptr)
        {
            if (beresta_last_error_code() != BERESTA_OK)
                return throwLastError(env);
            return makeNull(env);
        }
        std::uint64_t address = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(handle));
        napi().createBigintUint64(env, address, &result);
        return result;
    }

    // Статус без результата: успех -> undefined / A status without a result: success -> undefined
    napi_value makeStatus(napi_env env, int status) noexcept
    {
        if (status != BERESTA_OK)
            return throwLastError(env);
        return makeUndefined(env);
    }

    // Статус и выходной дескриптор: успех -> BigInt / A status and an output handle: success -> BigInt
    napi_value makeOut(napi_env env, int status, void* handle) noexcept
    {
        if (status != BERESTA_OK)
            return throwLastError(env);
        return makeHandle(env, handle);
    }

    // Значение null / The null value
    napi_value makeNull(napi_env env) noexcept
    {
        napi_value result = nullptr;

        napi().getNull(env, &result);
        return result;
    }

    // Значение undefined / The undefined value
    napi_value makeUndefined(napi_env env) noexcept
    {
        napi_value result = nullptr;

        napi().getUndefined(env, &result);
        return result;
    }
}
