#include "core/element.hpp"
#include "utils/qualified_name.hpp"

namespace beresta
{
    // Префикс — часть имени до двоеточия
    // The prefix is the part of the name before the colon
    std::string_view Element::getPrefix() const noexcept
    {
        return utils::splitQualifiedName(_name).prefix;
    }

    // Локальное имя — часть имени после двоеточия (или всё имя)
    // The local name is the part of the name after the colon (or the whole name)
    std::string_view Element::getLocalName() const noexcept
    {
        return utils::splitQualifiedName(_name).localName;
    }

    // Пространство имён элемента определяется его собственным префиксом
    // The namespace of an element is determined by its own prefix
    std::optional<std::string> Element::getNamespaceURI() const
    {
        return lookupNamespaceURI(getPrefix());
    }

    /*
    ** Ищем ближайшее объявление xmlns / xmlns:prefix, поднимаясь от элемента
    ** к корню. Префиксы xml и xmlns связаны со своими URI всегда.
    ** Пустое значение (xmlns="") отменяет пространство по умолчанию.
    **
    ** Looks for the nearest xmlns / xmlns:prefix declaration, climbing from the element
    ** to the root. The xml and xmlns prefixes are always bound to their URIs.
    ** An empty value (xmlns="") cancels the default namespace.
    */
    std::optional<std::string> Element::lookupNamespaceURI(std::string_view prefix) const
    {
        if (prefix == "xml")
            return std::string(XML_NAMESPACE_URI);
        if (prefix == "xmlns")
            return std::string(XMLNS_NAMESPACE_URI);
        for (const Node* node = this; node != nullptr; node = node->getParent())
        {
            if (node->getType() != NodeType::ELEMENT)
                break;
            for (const Attribute& attribute : static_cast<const Element*>(node)->_attributes)
            {
                if (!utils::declaresPrefix(attribute.name, prefix))
                    continue;
                if (attribute.value.empty())
                    return std::nullopt;
                return attribute.value;
            }
        }
        return std::nullopt;
    }

    /*
    ** Обратный поиск: префикс, под которым URI доступен в этой точке.
    ** Найденный префикс проверяется прямым поиском — он мог быть
    ** переопределён ближе к элементу другим URI.
    **
    ** The reverse lookup: the prefix under which the URI is available at this point.
    ** The found prefix is checked by a direct lookup — it might have been
    ** redefined closer to the element with another URI.
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
