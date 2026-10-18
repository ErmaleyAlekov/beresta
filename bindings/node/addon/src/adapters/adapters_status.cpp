#include "addon/adapters.hpp"
#include "addon/call_frame.hpp"
#include "addon/results.hpp"

namespace beresta::node
{
    // int f(handle) -> статус: удаление узла / int f(handle) -> a status: removing a node
    napi_value adaptStatusOfHandle(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;

        if (!call.isValid() || !call.handle(0, subject))
            return nullptr;
        StatusOfHandle function = reinterpret_cast<StatusOfHandle>(call.target());
        return makeStatus(env, function(subject));
    }

    // int f(handle) -> число: тип узла / int f(handle) -> a number: the node type
    napi_value adaptIntegerOfHandle(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;

        if (!call.isValid() || !call.handle(0, subject))
            return nullptr;
        StatusOfHandle function = reinterpret_cast<StatusOfHandle>(call.target());
        return makeInteger(env, function(subject));
    }

    // int f(handle, string) -> статус: замена текста / int f(handle, string) -> a status: replacing the text
    napi_value adaptStatusOfHandleString(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        ArgString text;

        if (!call.isValid() || !call.handle(0, subject) || !call.string(1, text))
            return nullptr;
        StatusOfHandleString function = reinterpret_cast<StatusOfHandleString>(call.target());
        return makeStatus(env, function(subject, text.get()));
    }

    // int f(handle, string, string): атрибут, префикс XPath, строковая переменная
    // int f(handle, string, string): an attribute, an XPath prefix, a string variable
    napi_value adaptStatusOfHandleStrings(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        ArgString first;
        ArgString second;

        if (!call.isValid() || !call.handle(0, subject) || !call.string(1, first) || !call.string(2, second))
            return nullptr;
        StatusOfHandleStrings function = reinterpret_cast<StatusOfHandleStrings>(call.target());
        return makeStatus(env, function(subject, first.get(), second.get()));
    }

    // int f(handle, string, double): числовая переменная XPath / int f(handle, string, double): a numeric XPath variable
    napi_value adaptStatusOfHandleStringNumber(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        ArgString name;
        double value = 0.0;

        if (!call.isValid() || !call.handle(0, subject) || !call.string(1, name) || !call.number(2, value))
            return nullptr;
        StatusOfHandleStringNumber function = reinterpret_cast<StatusOfHandleStringNumber>(call.target());
        return makeStatus(env, function(subject, name.get(), value));
    }
}
