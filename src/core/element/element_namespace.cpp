#include "core/element.hpp"
#include "utils/qualified_name.hpp"

namespace beresta
{
    // Префикс — часть имени до двоеточия
    std::string_view Element::getPrefix() const noexcept
    {
        return utils::splitQualifiedName(_name).prefix;
    }

    // Локальное имя — часть имени после двоеточия (или всё имя)
    std::string_view Element::getLocalName() const noexcept
    {
        return utils::splitQualifiedName(_name).localName;
    }

    // Пространство имён элемента определяется его собственным префиксом
    std::optional<std::string> Element::getNamespaceURI() const
    {
        return lookupNamespaceURI(getPrefix());
    }

    /*
    ** Ищем ближайшее объявление xmlns / xmlns:prefix, поднимаясь от элемента
    ** к корню. Префиксы xml и xmlns связаны со своими URI всегда.
    ** Пустое значение (xmlns="") отменяет пространство по умолчанию.
    */
    std::optional<std::string> Element::lookupNamespaceURI(std::string_view prefix) const
    {
        if (prefix == "xml")
            return std::string(XML_NAMESPACE_URI);
        if (prefix == "xmlns")
            return std::string(XMLNS_NAMESPACE_URI);
        std::string declaration = "xmlns";
        if (!prefix.empty())
            declaration += ":" + std::string(prefix);
        for (const Node* node = this; node != nullptr; node = node->getParent())
        {
            if (node->getType() != NodeType::ELEMENT)
                break;
            std::optional<std::string> value =
                static_cast<const Element*>(node)->getAttribute(declaration);
            if (value.has_value() && value->empty())
                return std::nullopt;
            if (value.has_value())
                return value;
        }
        return std::nullopt;
    }

    /*
    ** Обратный поиск: префикс, под которым URI доступен в этой точке.
    ** Найденный префикс проверяется прямым поиском — он мог быть
    ** переопределён ближе к элементу другим URI.
    */
    std::optional<std::string> Element::lookupPrefix(std::string_view namespaceUri) const
    {
        for (const Node* node = this; node != nullptr; node = node->getParent())
        {
            if (node->getType() != NodeType::ELEMENT)
                break;
            for (const Attribute& attribute : static_cast<const Element*>(node)->_attributes)
            {
                if (!utils::isNamespaceDeclaration(attribute.name) || attribute.value != namespaceUri)
                    continue;
                std::string_view prefix = utils::declaredPrefix(attribute.name);
                if (lookupNamespaceURI(prefix) == attribute.value)
                    return std::string(prefix);
            }
        }
        return std::nullopt;
    }
}
