#include "capi/internal/capi_support.hpp"

namespace
{
    // Атрибут по индексу с проверкой диапазона
    const beresta::Attribute& attributeAt(beresta_node element, int64_t index)
    {
        const std::vector<beresta::Attribute>& attributes = beresta::capi::toElement(element).getAttributes();

        return attributes[beresta::capi::checkedIndex(index, attributes.size())];
    }
}

// Полное имя атрибута с префиксом, например "xml:lang"
extern "C" BERESTA_API char* beresta_element_attribute_name(beresta_node element, int64_t index)
{
    BERESTA_CAPI_TRY
        return beresta::capi::duplicateString(attributeAt(element, index).name);
    BERESTA_CAPI_CATCH(nullptr)
}

extern "C" BERESTA_API char* beresta_element_attribute_value(beresta_node element, int64_t index)
{
    BERESTA_CAPI_TRY
        return beresta::capi::duplicateString(attributeAt(element, index).value);
    BERESTA_CAPI_CATCH(nullptr)
}

// Имя без префикса: у <s:item> — "item"
extern "C" BERESTA_API char* beresta_element_local_name(beresta_node element)
{
    BERESTA_CAPI_TRY
        return beresta::capi::duplicateString(beresta::capi::toElement(element).getLocalName());
    BERESTA_CAPI_CATCH(nullptr)
}

// URI пространства имён по объявлениям xmlns предков; NULL — вне пространств
extern "C" BERESTA_API char* beresta_element_namespace_uri(beresta_node element)
{
    BERESTA_CAPI_TRY
        std::optional<std::string> uri = beresta::capi::toElement(element).getNamespaceURI();

        if (!uri)
            return nullptr;
        return beresta::capi::duplicateString(*uri);
    BERESTA_CAPI_CATCH(nullptr)
}
