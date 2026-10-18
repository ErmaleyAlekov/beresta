#ifndef BERESTA_NODE_NAPI_ABI_HPP
#define BERESTA_NODE_NAPI_ABI_HPP

#include <cstddef>
#include <cstdint>

/*
** Собственное описание ABI Node-API (N-API). Node-API — стабильный
** двоичный интерфейс Node.js: функции с этими именами и сигнатурами
** экспортирует сам исполняемый файл node (и Electron), и они не меняются
** между версиями. Поэтому аддону не нужны ни заголовки Node, ни node-gyp,
** ни библиотека импорта node.lib: нужные функции находятся во время
** загрузки по имени (napi_loader.cpp). Описаны только используемые функции.
**
** Our own description of the Node-API (N-API) ABI. Node-API is the stable
** binary interface of Node.js: functions with these names and signatures
** are exported by the node executable itself (and by Electron), and they do
** not change between versions. So the addon needs neither the Node headers,
** nor node-gyp, nor the node.lib import library: the required functions are
** looked up by name at load time (napi_loader.cpp). Only the functions in use
** are described.
*/
extern "C"
{
    typedef struct napi_env__* napi_env;
    typedef struct napi_value__* napi_value;
    typedef struct napi_ref__* napi_ref;
    typedef struct napi_callback_info__* napi_callback_info;

    // Код состояния: 0 (napi_ok) — успех / The status code: 0 (napi_ok) is success
    typedef int napi_status;

    // Функция, вызываемая из JavaScript / A function called from JavaScript
    typedef napi_value (*napi_callback)(napi_env env, napi_callback_info info);

    // Освобождение данных при уничтожении окружения / Releases data when the environment is destroyed
    typedef void (*napi_finalize)(napi_env env, void* data, void* hint);
}

namespace beresta::node
{
    // Успешный код состояния Node-API / The successful Node-API status code
    constexpr napi_status NAPI_OK = 0;

    // Длина строки «до завершающего нуля» / The string length "up to the terminating zero"
    constexpr std::size_t NAPI_AUTO_LENGTH = SIZE_MAX;

    // Типы значений JavaScript (napi_valuetype) / JavaScript value types (napi_valuetype)
    enum class ValueType : int
    {
        UNDEFINED = 0, NULL_VALUE, BOOLEAN, NUMBER, STRING, SYMBOL, OBJECT, FUNCTION, EXTERNAL, BIGINT
    };

    // ---- Сигнатуры функций Node-API / Node-API function signatures ----
    using GetCbInfo = napi_status (*)(napi_env, napi_callback_info, std::size_t*, napi_value*, napi_value*, void**);
    using CreateFunction = napi_status (*)(napi_env, const char*, std::size_t, napi_callback, void*, napi_value*);
    using SetNamedProperty = napi_status (*)(napi_env, napi_value, const char*, napi_value);
    using TypeOf = napi_status (*)(napi_env, napi_value, int*);
    using GetBigintUint64 = napi_status (*)(napi_env, napi_value, std::uint64_t*, bool*);
    using CreateBigintUint64 = napi_status (*)(napi_env, std::uint64_t, napi_value*);
    using GetStringUtf8 = napi_status (*)(napi_env, napi_value, char*, std::size_t, std::size_t*);
    using CreateStringUtf8 = napi_status (*)(napi_env, const char*, std::size_t, napi_value*);
    using GetDouble = napi_status (*)(napi_env, napi_value, double*);
    using GetInt64 = napi_status (*)(napi_env, napi_value, std::int64_t*);
    using GetUint32 = napi_status (*)(napi_env, napi_value, std::uint32_t*);
    using CreateDouble = napi_status (*)(napi_env, double, napi_value*);
    using GetBoolean = napi_status (*)(napi_env, bool, napi_value*);
    using GetSingleton = napi_status (*)(napi_env, napi_value*);
    using IsTypedArray = napi_status (*)(napi_env, napi_value, bool*);
    using GetTypedArrayInfo = napi_status (*)(napi_env, napi_value, int*, std::size_t*, void**, napi_value*,
        std::size_t*);
    using CreateReference = napi_status (*)(napi_env, napi_value, std::uint32_t, napi_ref*);
    using DeleteReference = napi_status (*)(napi_env, napi_ref);
    using GetReferenceValue = napi_status (*)(napi_env, napi_ref, napi_value*);
    using NewInstance = napi_status (*)(napi_env, napi_value, std::size_t, const napi_value*, napi_value*);
    using Throw = napi_status (*)(napi_env, napi_value);
    using ThrowMessage = napi_status (*)(napi_env, const char*, const char*);
    using SetInstanceData = napi_status (*)(napi_env, void*, napi_finalize, void*);
    using GetInstanceData = napi_status (*)(napi_env, void**);

    /*
    ** Таблица найденных функций Node-API. Заполняется один раз при первой
    ** загрузке аддона и дальше только читается (одна на весь процесс:
    ** адреса функций одинаковы для всех потоков-воркеров).
    **
    ** The table of the found Node-API functions. It is filled once on the
    ** first load of the addon and only read afterwards (one per process:
    ** the function addresses are the same for all worker threads).
    */
    struct NapiTable
    {
        GetCbInfo getCbInfo;
        CreateFunction createFunction;
        SetNamedProperty setNamedProperty;
        TypeOf typeOf;
        GetBigintUint64 getBigintUint64;
        CreateBigintUint64 createBigintUint64;
        GetStringUtf8 getStringUtf8;
        CreateStringUtf8 createStringUtf8;
        GetDouble getDouble;
        GetInt64 getInt64;
        GetUint32 getUint32;
        CreateDouble createDouble;
        GetBoolean getBoolean;
        GetSingleton getNull;
        GetSingleton getUndefined;
        IsTypedArray isTypedArray;
        GetTypedArrayInfo getTypedArrayInfo;
        CreateReference createReference;
        DeleteReference deleteReference;
        GetReferenceValue getReferenceValue;
        NewInstance newInstance;
        Throw throwValue;
        ThrowMessage throwError;
        ThrowMessage throwTypeError;
        SetInstanceData setInstanceData;
        GetInstanceData getInstanceData;
    };

    // Таблица функций (действительна после успешного loadNapi) / The function table (valid after a successful loadNapi)
    const NapiTable& napi() noexcept;

    // Поиск всех функций в процессе Node; false — это не процесс Node
    // Looks up all functions in the Node process; false means this is not a Node process
    bool loadNapi() noexcept;
}

#endif
