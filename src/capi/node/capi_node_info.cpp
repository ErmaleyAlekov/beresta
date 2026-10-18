#include "capi/internal/capi_support.hpp"
#include "i18n/i18n.hpp"
#include "core/document_type.hpp"
#include "core/processing_instruction.hpp"

// Порядок значений в beresta_node_kind обязан совпадать с NodeType
// The order of values in beresta_node_kind must match NodeType
static_assert(static_cast<int>(beresta::NodeType::ELEMENT) == BERESTA_NODE_ELEMENT,
    "beresta_node_kind расходится с beresta::NodeType / beresta_node_kind does not match beresta::NodeType");
static_assert(static_cast<int>(beresta::NodeType::PROCESSING_INSTRUCTION) == BERESTA_NODE_PROCESSING_INSTRUCTION,
    "beresta_node_kind расходится с beresta::NodeType / beresta_node_kind does not match beresta::NodeType");

namespace
{
    /*
    ** Имя узла по соглашению DOM (nodeName): у элемента — тег, у PI —
    ** цель, у DOCTYPE — имя корня; у безымянных узлов — служебное имя
    ** с решёткой, которое не может совпасть ни с одним XML-именем.
    **
    ** The node name by the DOM convention (nodeName): for an element — the tag, for a PI —
    ** the target, for a DOCTYPE — the root name; for unnamed nodes — a service name
    ** with a hash sign that cannot match any XML name.
    */
    std::string nodeName(const beresta::Node& node)
    {
        beresta::NodeType type = node.getType();

        if (type == beresta::NodeType::ELEMENT)
            return static_cast<const beresta::Element&>(node).getName();
        if (type == beresta::NodeType::PROCESSING_INSTRUCTION)
            return static_cast<const beresta::ProcessingInstruction&>(node).getTarget();
        if (type == beresta::NodeType::DOCUMENT_TYPE)
            return static_cast<const beresta::DocumentType&>(node).getName();
        if (type == beresta::NodeType::TEXT)
            return "#text";
        if (type == beresta::NodeType::CDATA_SECTION)
            return "#cdata-section";
        if (type == beresta::NodeType::COMMENT)
            return "#comment";
        return "#document";
    }
}

// Тип узла; -1 — ошибка (например, передан NULL)
// The node type; -1 — an error (for example, NULL was passed)
extern "C" BERESTA_API int beresta_node_type(beresta_node node)
{
    BERESTA_CAPI_TRY
        return static_cast<int>(beresta::capi::toNode(node).getType());
    BERESTA_CAPI_CATCH(-1)
}

extern "C" BERESTA_API char* beresta_node_name(beresta_node node)
{
    BERESTA_CAPI_TRY
        return beresta::capi::duplicateString(nodeName(beresta::capi::toNode(node)));
    BERESTA_CAPI_CATCH(nullptr)
}

// Текстовое содержимое (getTextContent: у элемента — текст всех потомков)
// The text content (getTextContent: for an element — the text of all descendants)
extern "C" BERESTA_API char* beresta_node_text(beresta_node node)
{
    BERESTA_CAPI_TRY
        return beresta::capi::duplicateString(beresta::capi::toNode(node).getTextContent());
    BERESTA_CAPI_CATCH(nullptr)
}

// Сериализация: всегда UTF-8, так как строка уходит в другой язык как текст
// Serialization: always UTF-8, since the string goes to another language as text
extern "C" BERESTA_API char* beresta_node_to_string(beresta_node node, uint32_t flags)
{
    BERESTA_CAPI_TRY
        const beresta::Node& source = beresta::capi::toNode(node);
        beresta::XmlSerializer serializer(beresta::capi::serializerOptions(flags, nullptr));

        return beresta::capi::duplicateString(serializer.toString(source));
    BERESTA_CAPI_CATCH(nullptr)
}
