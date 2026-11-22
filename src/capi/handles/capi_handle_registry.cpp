#include "capi/internal/capi_support.hpp"

#include <mutex>
#include <typeindex>
#include <unordered_map>

namespace beresta::capi
{
    namespace
    {
        struct Registry
        {
            std::mutex mutex;
            std::unordered_map<void*, std::type_index> types;
        };

        Registry& registry()
        {
            static Registry instance;
            return instance;
        }
    }

    void registerHandle(void* pointer, const std::type_info& type)
    {
        Registry& entries = registry();
        std::lock_guard<std::mutex> lock(entries.mutex);
        entries.types.emplace(pointer, type);
    }

    bool isRegisteredHandle(void* pointer)
    {
        Registry& entries = registry();
        std::lock_guard<std::mutex> lock(entries.mutex);
        return entries.types.find(pointer) != entries.types.end();
    }

    // Проверка типа и удаление записи при освобождении выполняются под общим замком.
    // Type checking and removal during release take place under the same lock.
    void checkHandle(void* pointer, const std::type_info& type, const char* name, bool remove)
    {
        if (pointer == nullptr)
            throw InvalidArgumentException(i18n::format(i18n::Message::ARGUMENT_NULL, {name}));
        Registry& entries = registry();
        std::lock_guard<std::mutex> lock(entries.mutex);
        auto entry = entries.types.find(pointer);
        if (entry == entries.types.end() || entry->second != std::type_index(type))
            throw InvalidArgumentException(i18n::format(i18n::Message::HANDLE_WRONG_TYPE, {name}));
        if (remove)
            entries.types.erase(entry);
    }

    void clearOutput(void** out)
    {
        if (out == nullptr)
            throw InvalidArgumentException(i18n::format(i18n::Message::ARGUMENT_NULL, {"out"}));
        *out = nullptr;
    }
}
