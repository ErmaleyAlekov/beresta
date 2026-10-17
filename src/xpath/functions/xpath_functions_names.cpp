#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_functions.hpp"
#include "core/processing_instruction.hpp"
#include "utils/qualified_name.hpp"
#include "utils/string_utils.hpp"

namespace beresta::xpath::detail
{
    // Полное имя узла: элемента, атрибута или цели PI (у остальных имени нет)
    static std::string qualifiedNameOf(const XPathItem& item)
    {
        if (item.attribute != nullptr)
            return item.attribute->name;
        if (item.node == nullptr)
            return std::string();
        if (item.node->getType() == NodeType::ELEMENT)
            return static_cast<const Element*>(item.node)->getName();
        if (item.node->getType() == NodeType::PROCESSING_INSTRUCTION)
            return static_cast<const ProcessingInstruction*>(item.node)->getTarget();
        return std::string();
    }

    // local-name(): имя без префикса
    XPathValue functionLocalName(const FunctionCall& call)
    {
        XPathItem item;

        if (!nodeArgument(call, item))
            return XPathValue::fromString("");
        std::string name = qualifiedNameOf(item);
        return XPathValue::fromString(std::string(utils::splitQualifiedName(name).localName));
    }

    // namespace-uri(): URI пространства имён элемента или атрибута
    XPathValue functionNamespaceUri(const FunctionCall& call)
    {
        XPathItem item;

        if (!nodeArgument(call, item))
            return XPathValue::fromString("");
        if (item.attribute != nullptr)
        {
            std::string_view prefix = utils::splitQualifiedName(item.attribute->name).prefix;
            if (prefix.empty())
                return XPathValue::fromString("");
            return XPathValue::fromString(item.owner->lookupNamespaceURI(prefix).value_or(""));
        }
        if (item.node == nullptr || item.node->getType() != NodeType::ELEMENT)
            return XPathValue::fromString("");
        return XPathValue::fromString(
            static_cast<const Element*>(item.node)->getNamespaceURI().value_or(""));
    }

    // name(): полное имя
    XPathValue functionName(const FunctionCall& call)
    {
        XPathItem item;

        if (!nodeArgument(call, item))
            return XPathValue::fromString("");
        return XPathValue::fromString(qualifiedNameOf(item));
    }

    /*
    ** lang('ru'): ближайший xml:lang у узла или его предков равен 'ru'
    ** или начинается с 'ru-' (без учёта регистра).
    */
    XPathValue functionLang(const FunctionCall& call)
    {
        std::string wanted = stringArgument(call, 0);
        const Node* node = call.context.item.node;

        if (call.context.item.attribute != nullptr)
            node = call.context.item.owner;
        for (; node != nullptr; node = node->getParent())
        {
            if (node->getType() != NodeType::ELEMENT)
                continue;
            std::optional<std::string> lang = static_cast<const Element*>(node)->getAttribute("xml:lang");
            if (!lang.has_value())
                continue;
            std::string_view head = std::string_view(lang.value()).substr(0, wanted.size());
            bool boundary = lang->size() == wanted.size()
                || (lang->size() > wanted.size() && (*lang)[wanted.size()] == '-');
            return XPathValue::fromBoolean(utils::equalsIgnoreCaseAscii(head, wanted) && boundary);
        }
        return XPathValue::fromBoolean(false);
    }
}
