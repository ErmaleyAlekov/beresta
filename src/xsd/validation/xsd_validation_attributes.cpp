#include "xsd/internal/validation_run.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Каждый атрибут экземпляра должен быть объявлен в типе (или допущен
    ** xs:anyAttribute). Объявления xmlns и атрибуты xsi:* — служебные.
    */
    void ValidationRun::validateAttributes(const Element& element, const ComplexType& type,
        const std::string& path)
    {
        for (const Attribute& attribute : element.getAttributes())
        {
            std::pair<std::string, std::string> key = attributeKey(element, attribute);
            if (utils::isNamespaceDeclaration(attribute.name) || key.first == XSI_NAMESPACE_URI)
                continue;
            const AttributeUse* use = nullptr;
            for (const AttributeUse& candidate : type.attributes)
            {
                if (candidate.name == key.second && candidate.namespaceUri == key.first)
                    use = &candidate;
            }
            if (use == nullptr && !type.anyAttribute)
                report(element, path, "атрибут \"" + attribute.name + "\" не объявлен в типе");
            else if (use != nullptr && use->prohibited)
                report(element, path, "атрибут \"" + attribute.name + "\" запрещён (use=\"prohibited\")");
            else if (use != nullptr)
                checkAttribute(element, *use, attribute.value, path);
        }
        checkRequiredAttributes(element, type, path);
    }

    // Значение атрибута: тип, учёт ID/IDREF, фиксированное значение
    void ValidationRun::checkAttribute(const Element& element, const AttributeUse& use,
        const std::string& value, const std::string& path)
    {
        std::optional<std::string> error = validateSimpleValue(*use.type, value, &element);

        if (error.has_value())
        {
            report(element, path, "атрибут \"" + use.name + "\": " + error.value());
            return;
        }
        recordIdentity(*use.type, value, element, path);
        if (use.fixedValue.has_value() && !valuesEqual(*use.type, value, use.fixedValue.value()))
            report(element, path, "атрибут \"" + use.name + "\" должен быть равен \""
                + use.fixedValue.value() + "\"");
    }

    // Обязательные атрибуты (use="required")
    void ValidationRun::checkRequiredAttributes(const Element& element, const ComplexType& type,
        const std::string& path)
    {
        for (const AttributeUse& use : type.attributes)
        {
            if (use.required && !element.hasAttributeNS(use.namespaceUri, use.name))
                report(element, path, "отсутствует обязательный атрибут \"" + use.name + "\"");
        }
    }

    // Атрибут без префикса не принадлежит пространству имён; с префиксом — по объявлению xmlns
    std::pair<std::string, std::string> attributeKey(const Element& element, const Attribute& attribute)
    {
        utils::QualifiedNameParts parts = utils::splitQualifiedName(attribute.name);

        if (parts.prefix.empty())
            return std::pair<std::string, std::string>("", std::string(parts.localName));
        return std::pair<std::string, std::string>(element.lookupNamespaceURI(parts.prefix).value_or(""),
            std::string(parts.localName));
    }

    // Сегмент пути ребёнка с номером среди одноимённых соседей, если их несколько
    std::string childPath(const std::string& parentPath, const Element& child)
    {
        std::size_t index = 0;
        std::size_t total = 0;

        for (const Node* node = child.getParent()->getFirstChild(); node != nullptr; node = node->getNextSibling())
        {
            if (node->getType() != NodeType::ELEMENT || static_cast<const Element*>(node)->getName() != child.getName())
                continue;
            ++total;
            if (node == &child)
                index = total;
        }
        std::string path = parentPath + "/" + child.getName();
        if (total > 1)
            path += "[" + std::to_string(index) + "]";
        return path;
    }
}
