#include "addon/adapters.hpp"
#include "addon/call_frame.hpp"
#include "addon/results.hpp"

namespace beresta::node
{
    // int f(handle, out): глубокая копия документа / a deep copy of a document
    napi_value adaptOutOfHandle(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        void* created = nullptr;

        if (!call.isValid() || !call.handle(0, subject))
            return nullptr;
        OutOfHandle function = reinterpret_cast<OutOfHandle>(call.target());
        int status = function(subject, &created);
        return makeOut(env, status, created);
    }

    // handle f(handle): корень, родитель, дети и соседи, документ-владелец
    // handle f(handle): the root, the parent, the children and siblings, the owner document
    napi_value adaptHandleOfHandle(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;

        if (!call.isValid() || !call.handle(0, subject))
            return nullptr;
        HandleOfHandle function = reinterpret_cast<HandleOfHandle>(call.target());
        return makeHandle(env, function(subject));
    }

    // handle f(handle, string): поиск по ID, добавление элемента, текста, комментария, CDATA
    // handle f(handle, string): lookup by ID, appending an element, text, a comment, CDATA
    napi_value adaptHandleOfHandleString(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        ArgString text;

        if (!call.isValid() || !call.handle(0, subject) || !call.string(1, text))
            return nullptr;
        HandleOfHandleString function = reinterpret_cast<HandleOfHandleString>(call.target());
        return makeHandle(env, function(subject, text.get()));
    }

    // handle f(handle, index): ребёнок по индексу, узел набора / a child by index, a set node
    napi_value adaptHandleOfHandleIndex(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        std::int64_t index = 0;

        if (!call.isValid() || !call.handle(0, subject) || !call.integer(1, index))
            return nullptr;
        HandleOfHandleIndex function = reinterpret_cast<HandleOfHandleIndex>(call.target());
        return makeHandle(env, function(subject, index));
    }

    // handle f(handle, string, string): инструкция обработки / a processing instruction
    napi_value adaptHandleOfHandleStrings(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        ArgString first;
        ArgString second;

        if (!call.isValid() || !call.handle(0, subject) || !call.string(1, first) || !call.string(2, second))
            return nullptr;
        HandleOfHandleStrings function = reinterpret_cast<HandleOfHandleStrings>(call.target());
        return makeHandle(env, function(subject, first.get(), second.get()));
    }
}
