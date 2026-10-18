#include "addon/adapters.hpp"
#include "addon/call_frame.hpp"
#include "addon/results.hpp"

namespace beresta::node
{
    // int f(handle, handle, out): копия поддерева, выборка XPath, проверка по схеме
    // int f(handle, handle, out): a subtree copy, an XPath selection, schema validation
    napi_value adaptOutOfHandles(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* first = nullptr;
        void* second = nullptr;
        void* created = nullptr;

        if (!call.isValid() || !call.handle(0, first) || !call.handle(1, second))
            return nullptr;
        OutOfHandles function = reinterpret_cast<OutOfHandles>(call.target());
        int status = function(first, second, &created);
        return makeOut(env, status, created);
    }

    /*
    ** document_parse(bytes, flags, schema): байты передаются без копирования,
    ** схема необязательна (null — без проверки по схеме).
    **
    ** document_parse(bytes, flags, schema): the bytes are passed without copying,
    ** the schema is optional (null means no schema validation).
    */
    napi_value adaptParse(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        ByteView bytes{nullptr, 0};
        std::uint32_t flags = 0;
        void* schema = nullptr;
        void* created = nullptr;

        if (!call.isValid() || !call.bytes(0, bytes) || !call.flags(1, flags) || !call.handle(2, schema))
            return nullptr;
        ParseBytes function = reinterpret_cast<ParseBytes>(call.target());
        int status = function(bytes.data, bytes.size, flags, schema, &created);
        return makeOut(env, status, created);
    }

    // document_parse_file(path, flags, schema) / document_parse_file(path, flags, schema)
    napi_value adaptParseFile(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        ArgString path;
        std::uint32_t flags = 0;
        void* schema = nullptr;
        void* created = nullptr;

        if (!call.isValid() || !call.string(0, path) || !call.flags(1, flags) || !call.handle(2, schema))
            return nullptr;
        ParseFile function = reinterpret_cast<ParseFile>(call.target());
        int status = function(path.get(), flags, schema, &created);
        return makeOut(env, status, created);
    }

    // document_save(document, path, flags, encoding); encoding null — UTF-8
    // document_save(document, path, flags, encoding); a null encoding means UTF-8
    napi_value adaptSave(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* document = nullptr;
        ArgString path;
        std::uint32_t flags = 0;
        ArgString encoding;

        if (!call.isValid() || !call.handle(0, document) || !call.string(1, path) || !call.flags(2, flags)
            || !call.string(3, encoding))
            return nullptr;
        SaveDocument function = reinterpret_cast<SaveDocument>(call.target());
        return makeStatus(env, function(document, path.get(), flags, encoding.get()));
    }

    // schema_load(bytes): схема из байтов XSD / schema_load(bytes): a schema from XSD bytes
    napi_value adaptLoadSchema(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        ByteView bytes{nullptr, 0};
        void* created = nullptr;

        if (!call.isValid() || !call.bytes(0, bytes))
            return nullptr;
        LoadSchema function = reinterpret_cast<LoadSchema>(call.target());
        int status = function(bytes.data, bytes.size, &created);
        return makeOut(env, status, created);
    }
}
