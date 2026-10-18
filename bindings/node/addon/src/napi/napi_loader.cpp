#include "addon/napi_symbols.hpp"

namespace beresta::node
{
    /*
    ** Находит функцию по имени и кладёт её в ячейку таблицы с настоящим
    ** типом. Отсутствие хотя бы одной функции отмечается в complete.
    **
    ** Finds a function by name and puts it into the table slot with its real
    ** type. A missing function, even a single one, is recorded in complete.
    */
    template <typename Function>
    static void resolve(Function& slot, const char* name, bool& complete) noexcept
    {
        GenericFunction found = findSymbol(name);

        if (found == nullptr)
            complete = false;
        slot = reinterpret_cast<Function>(found);
    }

    // Функции значений: числа, строки, BigInt, типизированные массивы
    // Value functions: numbers, strings, BigInt, typed arrays
    static void resolveValues(NapiTable& table, bool& complete) noexcept
    {
        resolve(table.typeOf, "napi_typeof", complete);
        resolve(table.getBigintUint64, "napi_get_value_bigint_uint64", complete);
        resolve(table.createBigintUint64, "napi_create_bigint_uint64", complete);
        resolve(table.getStringUtf8, "napi_get_value_string_utf8", complete);
        resolve(table.createStringUtf8, "napi_create_string_utf8", complete);
        resolve(table.getDouble, "napi_get_value_double", complete);
        resolve(table.getInt64, "napi_get_value_int64", complete);
        resolve(table.getUint32, "napi_get_value_uint32", complete);
        resolve(table.createDouble, "napi_create_double", complete);
        resolve(table.getBoolean, "napi_get_boolean", complete);
        resolve(table.getNull, "napi_get_null", complete);
        resolve(table.getUndefined, "napi_get_undefined", complete);
        resolve(table.isTypedArray, "napi_is_typedarray", complete);
        resolve(table.getTypedArrayInfo, "napi_get_typedarray_info", complete);
    }

    // Функции вызовов, ссылок, исключений и данных окружения
    // Functions of calls, references, exceptions and environment data
    static void resolveRuntime(NapiTable& table, bool& complete) noexcept
    {
        resolve(table.getCbInfo, "napi_get_cb_info", complete);
        resolve(table.createFunction, "napi_create_function", complete);
        resolve(table.setNamedProperty, "napi_set_named_property", complete);
        resolve(table.createReference, "napi_create_reference", complete);
        resolve(table.deleteReference, "napi_delete_reference", complete);
        resolve(table.getReferenceValue, "napi_get_reference_value", complete);
        resolve(table.newInstance, "napi_new_instance", complete);
        resolve(table.throwValue, "napi_throw", complete);
        resolve(table.throwError, "napi_throw_error", complete);
        resolve(table.throwTypeError, "napi_throw_type_error", complete);
        resolve(table.setInstanceData, "napi_set_instance_data", complete);
        resolve(table.getInstanceData, "napi_get_instance_data", complete);
    }

    // Полное заполнение таблицы / Fills the whole table
    static bool loadAll() noexcept
    {
        bool complete = true;

        resolveValues(mutableNapi(), complete);
        resolveRuntime(mutableNapi(), complete);
        return complete;
    }

    /*
    ** Таблица заполняется ровно один раз: инициализация локальной
    ** статической переменной в C++11 и новее потокобезопасна, поэтому
    ** одновременная загрузка аддона из нескольких воркеров безопасна.
    **
    ** The table is filled exactly once: initialization of a local static
    ** variable in C++11 and later is thread-safe, so loading the addon
    ** from several workers at the same time is safe.
    */
    bool loadNapi() noexcept
    {
        static const bool loaded = loadAll();

        return loaded;
    }
}
