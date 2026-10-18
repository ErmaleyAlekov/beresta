#include "addon/adapters.hpp"
#include "addon/call_frame.hpp"
#include "addon/results.hpp"

namespace beresta::node
{
    // const char* f(void): версия, язык, список языков / version, language, the language list
    napi_value adaptBorrowed(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);

        if (!call.isValid())
            return nullptr;
        BorrowedOfNothing function = reinterpret_cast<BorrowedOfNothing>(call.target());
        return makeBorrowedString(env, function());
    }

    // const char* f(int): имя кода состояния, текст сообщения / a status code name, a message text
    napi_value adaptBorrowedOfInt(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        std::int64_t value = 0;

        if (!call.isValid() || !call.integer(0, value))
            return nullptr;
        BorrowedOfInt function = reinterpret_cast<BorrowedOfInt>(call.target());
        return makeBorrowedString(env, function(static_cast<int>(value)));
    }

    // int f(const char*): смена языка / switching the language
    napi_value adaptStatusOfString(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        ArgString text;

        if (!call.isValid() || !call.string(0, text))
            return nullptr;
        StatusOfString function = reinterpret_cast<StatusOfString>(call.target());
        return makeStatus(env, function(text.get()));
    }

    // int f(const char*, out): новый документ, компиляция XPath, схема из файла
    // int f(const char*, out): a new document, XPath compilation, a schema from a file
    napi_value adaptOutOfString(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        ArgString text;
        void* created = nullptr;

        if (!call.isValid() || !call.string(0, text))
            return nullptr;
        OutOfString function = reinterpret_cast<OutOfString>(call.target());
        int status = function(text.get(), &created);
        return makeOut(env, status, created);
    }

    // void f(handle): освобождение документа, выражения, набора, схемы, списка
    // void f(handle): freeing a document, an expression, a set, a schema, a list
    napi_value adaptVoidOfHandle(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;

        if (!call.isValid() || !call.handle(0, subject))
            return nullptr;
        VoidOfHandle function = reinterpret_cast<VoidOfHandle>(call.target());
        function(subject);
        return makeUndefined(env);
    }
}
