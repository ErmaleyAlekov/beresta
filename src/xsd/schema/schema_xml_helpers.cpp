#include "xsd/internal/schema_builder.hpp"

namespace beresta::xsd::detail
{
    // Элемент из пространства XSD с нужным локальным именем
    // An element from the XSD namespace with the required local name
    bool isSchemaElement(const Element& element, std::string_view localName)
    {
        return element.getLocalName() == localName
            && element.getNamespaceURI().value_or("") == XSD_NAMESPACE_URI;
    }

    // Компоненты схемы — дочерние элементы XSD; аннотации (документация) пропускаются
    // Schema components are XSD child elements; annotations (documentation) are skipped
    std::vector<const Element*> schemaChildren(const Element& parent)
    {
        std::vector<const Element*> children;

        for (const Node* child = parent.getFirstChild(); child != nullptr; child = child->getNextSibling())
        {
            if (child->getType() != NodeType::ELEMENT)
                continue;
            const Element* element = static_cast<const Element*>(child);
            if (element->getNamespaceURI().value_or("") != XSD_NAMESPACE_URI)
                continue;
            if (element->getLocalName() != "annotation")
                children.push_back(element);
        }
        return children;
    }

    // Первый ребёнок XSD с указанным именем
    // The first XSD child with the given name
    const Element* firstSchemaChild(const Element& parent, std::string_view localName)
    {
        for (const Element* child : schemaChildren(parent))
        {
            if (child->getLocalName() == localName)
                return child;
        }
        return nullptr;
    }

    // Значение атрибута без пространства имён или запасное значение
    // The value of an attribute without a namespace or a fallback value
    std::string attributeOr(const Element& element, std::string_view name, std::string_view fallback)
    {
        std::optional<std::string> value = element.getAttribute(name);

        if (!value.has_value())
            return std::string(fallback);
        return normalizeWhiteSpace(value.value(), WhiteSpace::COLLAPSE);
    }

    // strict | lax | skip
    ProcessContents parseProcessContents(const Element& wildcard)
    {
        std::string process = attributeOr(wildcard, "processContents", "strict");

        if (process == "lax")
            return ProcessContents::LAX_CHECK;
        if (process == "skip")
            return ProcessContents::SKIP_CHECK;
        return ProcessContents::STRICT_CHECK;
    }
}
