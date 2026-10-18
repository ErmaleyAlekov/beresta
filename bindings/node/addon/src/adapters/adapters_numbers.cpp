#include "addon/adapters.hpp"
#include "addon/call_frame.hpp"
#include "addon/results.hpp"

namespace beresta::node
{
    // int64 f(handle): число детей, атрибутов, узлов набора, диагностик
    // int64 f(handle): the number of children, attributes, set nodes, diagnostics
    napi_value adaptCountOfHandle(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;

        if (!call.isValid() || !call.handle(0, subject))
            return nullptr;
        CountOfHandle function = reinterpret_cast<CountOfHandle>(call.target());
        return makeCount(env, function(subject));
    }

    // int64 f(handle, index): строка и столбец диагностики / int64 f(handle, index): a diagnostic line and column
    napi_value adaptCountOfHandleIndex(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        std::int64_t index = 0;

        if (!call.isValid() || !call.handle(0, subject) || !call.integer(1, index))
            return nullptr;
        CountOfHandleIndex function = reinterpret_cast<CountOfHandleIndex>(call.target());
        return makeCount(env, function(subject, index));
    }

    // int f(handle, string) -> true/false: наличие и удаление атрибута
    // int f(handle, string) -> true/false: checking for and removing an attribute
    napi_value adaptFlagOfHandleString(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        ArgString name;

        if (!call.isValid() || !call.handle(0, subject) || !call.string(1, name))
            return nullptr;
        StatusOfHandleString function = reinterpret_cast<StatusOfHandleString>(call.target());
        return makeFlag(env, function(subject, name.get()));
    }

    // int f(handle, handle) -> true/false: результат XPath логическим значением
    // int f(handle, handle) -> true/false: an XPath result as a boolean
    napi_value adaptFlagOfHandles(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* first = nullptr;
        void* second = nullptr;

        if (!call.isValid() || !call.handle(0, first) || !call.handle(1, second))
            return nullptr;
        FlagOfHandles function = reinterpret_cast<FlagOfHandles>(call.target());
        return makeFlag(env, function(first, second));
    }

    // int f(handle, handle, double* out): результат XPath числом / an XPath result as a number
    napi_value adaptNumberOfHandles(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* first = nullptr;
        void* second = nullptr;
        double value = 0.0;

        if (!call.isValid() || !call.handle(0, first) || !call.handle(1, second))
            return nullptr;
        NumberOfHandles function = reinterpret_cast<NumberOfHandles>(call.target());
        int status = function(first, second, &value);
        if (status != 0)
            return makeStatus(env, status);
        return makeNumber(env, value);
    }
}
