#include "addon/napi_symbols.hpp"

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace beresta::node
{
    /*
    ** Единственная таблица функций Node-API на процесс. Нулевая инициализация
    ** статического объекта гарантирована до любого обращения к нему.
    **
    ** The single Node-API function table per process. Zero initialization
    ** of a static object is guaranteed before any access to it.
    */
    static NapiTable table;

    // Таблица только для чтения / The read-only table
    const NapiTable& napi() noexcept
    {
        return table;
    }

    // Таблица для заполнения загрузчиком / The table for the loader to fill
    NapiTable& mutableNapi() noexcept
    {
        return table;
    }

#if defined(_WIN32)
    /*
    ** Windows: функции Node-API экспортирует исполняемый файл процесса
    ** (node.exe, electron.exe). Встраиваемые сборки Node кладут их в
    ** node.dll / libnode.dll — эти модули проверяются следом. Приведение
    ** FARPROC к GenericFunction — допустимое приведение между функциями.
    **
    ** Windows: the Node-API functions are exported by the process executable
    ** (node.exe, electron.exe). Embedded Node builds put them into
    ** node.dll / libnode.dll — these modules are checked next. Casting
    ** FARPROC to GenericFunction is an allowed cast between functions.
    */
    GenericFunction findSymbol(const char* name) noexcept
    {
        const char* modules[] = {nullptr, "node.dll", "libnode.dll"};

        for (const char* module : modules)
        {
            HMODULE handle = GetModuleHandleA(module);
            if (handle == nullptr)
                continue;
            FARPROC found = GetProcAddress(handle, name);
            if (found != nullptr)
                return reinterpret_cast<GenericFunction>(found);
        }
        return nullptr;
    }
#else
    /*
    ** Linux и macOS: исполняемый файл node экспортирует функции Node-API
    ** в глобальную область символов, и RTLD_DEFAULT находит их там.
    **
    ** Linux and macOS: the node executable exports the Node-API functions
    ** into the global symbol scope, and RTLD_DEFAULT finds them there.
    */
    GenericFunction findSymbol(const char* name) noexcept
    {
        void* found = dlsym(RTLD_DEFAULT, name);

        return reinterpret_cast<GenericFunction>(found);
    }
#endif
}
