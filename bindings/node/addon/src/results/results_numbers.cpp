#include "addon/results.hpp"
#include "addon/errors.hpp"

namespace beresta::node
{
    // Размер или индекс: -1 означает ошибку / A size or an index: -1 means an error
    napi_value makeCount(napi_env env, std::int64_t value) noexcept
    {
        if (value < 0)
            return throwLastError(env);
        return makeNumber(env, static_cast<double>(value));
    }

    // 1 — true, 0 — false, -1 — ошибка / 1 — true, 0 — false, -1 — an error
    napi_value makeFlag(napi_env env, int value) noexcept
    {
        napi_value result = nullptr;

        if (value < 0)
            return throwLastError(env);
        napi().getBoolean(env, value != 0, &result);
        return result;
    }

    // Целое значение (тип узла); -1 — ошибка / An integer value (the node type); -1 — an error
    napi_value makeInteger(napi_env env, int value) noexcept
    {
        if (value < 0)
            return throwLastError(env);
        return makeNumber(env, static_cast<double>(value));
    }

    // Число JavaScript; NaN и бесконечности передаются как есть / A JavaScript number; NaN and infinities pass as is
    napi_value makeNumber(napi_env env, double value) noexcept
    {
        napi_value result = nullptr;

        napi().createDouble(env, value, &result);
        return result;
    }
}
