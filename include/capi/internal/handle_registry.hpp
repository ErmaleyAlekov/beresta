#ifndef BERESTA_HANDLE_REGISTRY_HPP
#define BERESTA_HANDLE_REGISTRY_HPP

#include <memory>
#include <typeinfo>

namespace beresta::capi
{
    // Узлы сохраняют исходный адрес Node*; в реестре только остальные типы.
    // Nodes retain their original Node* address; only other types are registered.
    void registerHandle(void* pointer, const std::type_info& type);
    bool isRegisteredHandle(void* pointer);
    void checkHandle(void* pointer, const std::type_info& type, const char* name, bool remove);
    void clearOutput(void** out);

    // Владение переходит вызывающему только после успешной регистрации.
    // Ownership passes to the caller only after registration succeeds.
    template <typename Handle, typename Deleter>
    Handle* publishHandle(std::unique_ptr<Handle, Deleter> owned)
    {
        registerHandle(owned.get(), typeid(Handle));
        return owned.release();
    }

    template <typename Handle>
    Handle& requireHandle(void* pointer, const char* name)
    {
        checkHandle(pointer, typeid(Handle), name, false);
        return *static_cast<Handle*>(pointer);
    }

    template <typename Handle>
    void freeHandle(void* pointer, const char* name)
    {
        if (pointer == nullptr)
            return;
        checkHandle(pointer, typeid(Handle), name, true);
        delete static_cast<Handle*>(pointer);
    }
}

#endif
