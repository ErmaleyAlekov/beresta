#include "addon/adapters.hpp"
#include "addon/call_frame.hpp"
#include "capi/beresta_c.h"

/*
** Строка таблицы: имя без префикса beresta_ (так функция видна в
** JavaScript), переходник её сигнатуры и адрес самой функции. Макрос
** избавляет от повторения имени трижды и от опечатки в одном из повторов.
**
** A table row: the name without the beresta_ prefix (this is how the
** function is visible in JavaScript), the adapter of its signature and the
** address of the function itself. The macro saves repeating the name three
** times and a typo in one of the repetitions.
*/
#define BERESTA_BIND(name, adapter) \
    {#name, adapter, reinterpret_cast<beresta::node::GenericFunction>(&beresta_##name)}

namespace beresta::node
{
    /*
    ** Все функции Си-интерфейса, нужные JavaScript. Последняя ошибка
    ** (last_error_*) и beresta_string_free наружу не выходят: ошибки
    ** аддон превращает в исключения сам, а строки освобождает сразу.
    **
    ** All C interface functions JavaScript needs. The last error
    ** (last_error_*) and beresta_string_free are not exposed: the addon
    ** turns errors into exceptions itself and frees strings at once.
    */
    static const Binding TABLE[] = {
        BERESTA_BIND(version, adaptBorrowed),
        BERESTA_BIND(status_name, adaptBorrowedOfInt),
        BERESTA_BIND(set_language, adaptStatusOfString),
        BERESTA_BIND(language, adaptBorrowed),
        BERESTA_BIND(languages, adaptBorrowed),
        BERESTA_BIND(message_text, adaptBorrowedOfInt),

        BERESTA_BIND(document_create, adaptOutOfString),
        BERESTA_BIND(document_parse, adaptParse),
        BERESTA_BIND(document_parse_file, adaptParseFile),
        BERESTA_BIND(document_free, adaptVoidOfHandle),
        BERESTA_BIND(document_clone, adaptOutOfHandle),
        BERESTA_BIND(document_root, adaptHandleOfHandle),
        BERESTA_BIND(document_element_by_id, adaptHandleOfHandleString),
        BERESTA_BIND(document_save, adaptSave),

        BERESTA_BIND(node_type, adaptIntegerOfHandle),
        BERESTA_BIND(node_name, adaptOwnedOfHandle),
        BERESTA_BIND(node_text, adaptOwnedOfHandle),
        BERESTA_BIND(node_to_string, adaptOwnedOfHandleFlags),
        BERESTA_BIND(node_parent, adaptHandleOfHandle),
        BERESTA_BIND(node_first_child, adaptHandleOfHandle),
        BERESTA_BIND(node_last_child, adaptHandleOfHandle),
        BERESTA_BIND(node_next_sibling, adaptHandleOfHandle),
        BERESTA_BIND(node_previous_sibling, adaptHandleOfHandle),
        BERESTA_BIND(node_owner_document, adaptHandleOfHandle),
        BERESTA_BIND(node_child_count, adaptCountOfHandle),
        BERESTA_BIND(node_child, adaptHandleOfHandleIndex),

        BERESTA_BIND(node_set_text, adaptStatusOfHandleString),
        BERESTA_BIND(node_remove, adaptStatusOfHandle),
        BERESTA_BIND(node_copy_to, adaptOutOfHandles),
        BERESTA_BIND(node_move_to, adaptStatusOfHandles),
        BERESTA_BIND(node_append_element, adaptHandleOfHandleString),
        BERESTA_BIND(node_append_text, adaptHandleOfHandleString),
        BERESTA_BIND(node_append_comment, adaptHandleOfHandleString),
        BERESTA_BIND(node_append_cdata, adaptHandleOfHandleString),
        BERESTA_BIND(node_append_pi, adaptHandleOfHandleStrings),

        BERESTA_BIND(element_get_attribute, adaptOwnedOfHandleString),
        BERESTA_BIND(element_set_attribute, adaptStatusOfHandleStrings),
        BERESTA_BIND(element_has_attribute, adaptFlagOfHandleString),
        BERESTA_BIND(element_remove_attribute, adaptFlagOfHandleString),
        BERESTA_BIND(element_attribute_count, adaptCountOfHandle),
        BERESTA_BIND(element_attribute_name, adaptOwnedOfHandleIndex),
        BERESTA_BIND(element_attribute_value, adaptOwnedOfHandleIndex),
        BERESTA_BIND(element_local_name, adaptOwnedOfHandle),
        BERESTA_BIND(element_namespace_uri, adaptOwnedOfHandle),

        BERESTA_BIND(xpath_compile, adaptOutOfString),
        BERESTA_BIND(xpath_free, adaptVoidOfHandle),
        BERESTA_BIND(xpath_bind_namespace, adaptStatusOfHandleStrings),
        BERESTA_BIND(xpath_set_string, adaptStatusOfHandleStrings),
        BERESTA_BIND(xpath_set_number, adaptStatusOfHandleStringNumber),
        BERESTA_BIND(xpath_select, adaptOutOfHandles),
        BERESTA_BIND(xpath_string, adaptOwnedOfHandles),
        BERESTA_BIND(xpath_number, adaptNumberOfHandles),
        BERESTA_BIND(xpath_boolean, adaptFlagOfHandles),
        BERESTA_BIND(nodeset_size, adaptCountOfHandle),
        BERESTA_BIND(nodeset_node, adaptHandleOfHandleIndex),
        BERESTA_BIND(nodeset_value, adaptOwnedOfHandleIndex),
        BERESTA_BIND(nodeset_free, adaptVoidOfHandle),

        BERESTA_BIND(schema_load, adaptLoadSchema),
        BERESTA_BIND(schema_load_file, adaptOutOfString),
        BERESTA_BIND(schema_free, adaptVoidOfHandle),
        BERESTA_BIND(schema_validate, adaptOutOfHandles),
        BERESTA_BIND(diagnostics_count, adaptCountOfHandle),
        BERESTA_BIND(diagnostics_message, adaptOwnedOfHandleIndex),
        BERESTA_BIND(diagnostics_line, adaptCountOfHandleIndex),
        BERESTA_BIND(diagnostics_column, adaptCountOfHandleIndex),
        BERESTA_BIND(diagnostics_free, adaptVoidOfHandle)
    };

    // Таблица и число её строк / The table and the number of its rows
    const Binding* bindingTable(std::size_t& count) noexcept
    {
        count = sizeof(TABLE) / sizeof(TABLE[0]);
        return TABLE;
    }
}
