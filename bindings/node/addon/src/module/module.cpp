#include "addon/call_frame.hpp"
#include "addon/errors.hpp"

#include <cstdint>

/*
** Видимость точек входа модуля. Аддон собирается со скрытой видимостью
** (-fvisibility=hidden), поэтому наружу явно выводятся только функции,
** которые ищет сам Node.
**
** The visibility of the module entry points. The addon is built with hidden
** visibility (-fvisibility=hidden), so only the functions Node itself looks
** for are exported explicitly.
*/
#if defined(_WIN32)
    #define BERESTA_NODE_EXPORT __declspec(dllexport)
#else
    #define BERESTA_NODE_EXPORT __attribute__((visibility("default")))
#endif

namespace beresta::node
{
    // Версия Node-API, на которую рассчитан аддон (BigInt и данные окружения — версия 6)
    // The Node-API version the addon targets (BigInt and environment data are version 6)
    static constexpr std::int32_t NODE_API_VERSION = 8;

    // Одна функция в объекте exports; data — описание для переходника
    // One function in the exports object; data is the description for the adapter
    static bool exportFunction(napi_env env, napi_value exports, const char* name, napi_callback callback,
        const void* data) noexcept
    {
        napi_value function = nullptr;

        if (napi().createFunction(env, name, NAPI_AUTO_LENGTH, callback, const_cast<void*>(data), &function) != NAPI_OK)
            return false;
        return napi().setNamedProperty(env, exports, name, function) == NAPI_OK;
    }

    /*
    ** Регистрация: служебная init и все функции таблицы. Описание Binding
    ** передаётся как данные функции: так переходник узнаёт, что вызывать.
    **
    ** Registration: the service init and all functions of the table. The Binding
    ** description is passed as the function data: this is how an adapter learns what to call.
    */
    static napi_value registerAll(napi_env env, napi_value exports) noexcept
    {
        std::size_t count = 0;
        const Binding* table = bindingTable(count);

        if (!exportFunction(env, exports, "init", initialize, nullptr))
            return exports;
        for (std::size_t index = 0; index < count; ++index)
            exportFunction(env, exports, table[index].name, table[index].adapter, &table[index]);
        return exports;
    }
}

/*
** Точка входа, которую Node ищет в любом аддоне Node-API. Если функции
** Node-API найти не удалось (библиотеку загрузили не из Node), объект
** exports возвращается пустым — загрузчик на JavaScript сообщит об этом.
**
** The entry point Node looks for in any Node-API addon. If the Node-API
** functions could not be found (the library was not loaded from Node), the
** exports object is returned empty — the JavaScript loader reports it.
*/
extern "C" BERESTA_NODE_EXPORT napi_value napi_register_module_v1(napi_env env, napi_value exports)
{
    if (!beresta::node::loadNapi())
        return exports;
    return beresta::node::registerAll(env, exports);
}

// Версия Node-API модуля (Node 21+ спрашивает её до регистрации)
// The Node-API version of the module (Node 21+ asks for it before registration)
extern "C" BERESTA_NODE_EXPORT std::int32_t node_api_module_get_api_version_v1(void)
{
    return beresta::node::NODE_API_VERSION;
}
