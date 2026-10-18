#include "addon/adapters.hpp"
#include "addon/call_frame.hpp"
#include "addon/results.hpp"

namespace beresta::node
{
    // char* f(handle): имя, текст, локальное имя, URI пространства имён
    // char* f(handle): the name, the text, the local name, the namespace URI
    napi_value adaptOwnedOfHandle(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;

        if (!call.isValid() || !call.handle(0, subject))
            return nullptr;
        OwnedOfHandle function = reinterpret_cast<OwnedOfHandle>(call.target());
        return makeOwnedString(env, function(subject));
    }

    // char* f(handle, flags): сериализация узла / char* f(handle, flags): serializing a node
    napi_value adaptOwnedOfHandleFlags(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        std::uint32_t flags = 0;

        if (!call.isValid() || !call.handle(0, subject) || !call.flags(1, flags))
            return nullptr;
        OwnedOfHandleFlags function = reinterpret_cast<OwnedOfHandleFlags>(call.target());
        return makeOwnedString(env, function(subject, flags));
    }

    // char* f(handle, string): значение атрибута / char* f(handle, string): an attribute value
    napi_value adaptOwnedOfHandleString(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        ArgString name;

        if (!call.isValid() || !call.handle(0, subject) || !call.string(1, name))
            return nullptr;
        OwnedOfHandleString function = reinterpret_cast<OwnedOfHandleString>(call.target());
        return makeOwnedString(env, function(subject, name.get()));
    }

    // char* f(handle, index): атрибут по индексу, значение набора, текст диагностики
    // char* f(handle, index): an attribute by index, a set value, a diagnostic text
    napi_value adaptOwnedOfHandleIndex(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        std::int64_t index = 0;

        if (!call.isValid() || !call.handle(0, subject) || !call.integer(1, index))
            return nullptr;
        OwnedOfHandleIndex function = reinterpret_cast<OwnedOfHandleIndex>(call.target());
        return makeOwnedString(env, function(subject, index));
    }

    // char* f(handle, handle): результат XPath строкой / char* f(handle, handle): an XPath result as a string
    napi_value adaptOwnedOfHandles(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* first = nullptr;
        void* second = nullptr;

        if (!call.isValid() || !call.handle(0, first) || !call.handle(1, second))
            return nullptr;
        OwnedOfHandles function = reinterpret_cast<OwnedOfHandles>(call.target());
        return makeOwnedString(env, function(first, second));
    }
}
