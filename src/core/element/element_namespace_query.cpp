#include "core/element.hpp"
#include "utils/qualified_name.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Пространство имён атрибута по правилам Namespaces in XML:
    ** - объявления xmlns / xmlns:* принадлежат служебному пространству XMLNS;
    ** - атрибут без префикса НЕ наследует пространство по умолчанию;
    ** - атрибут с префиксом — в пространстве, связанном с префиксом.
    **
    ** The namespace of an attribute by the Namespaces in XML rules:
    **  - xmlns / xmlns:* declarations belong to the service XMLNS namespace;
    **  - an attribute without a prefix does NOT inherit the default namespace;
    **  - an attribute with a prefix is in the namespace bound to the prefix.
    */
    std::optional<std::string> Element::attributeNamespace(const Attribute& attribute) const
    {
        if (utils::isNamespaceDeclaration(attribute.name))
            return std::string(XMLNS_NAMESPACE_URI);
        utils::QualifiedNameParts parts = utils::splitQualifiedName(attribute.name);
        if (parts.prefix.empty())
            return std::nullopt;
        return lookupNamespaceURI(parts.prefix);
    }

    // Пустой namespaceUri означает «атрибут вне пространств имён»
    // An empty namespaceUri means "the attribute is in no namespace"
    std::size_t Element::findAttributeNS(std::string_view namespaceUri,
        std::string_view localName) const
    {
        for (std::size_t index = 0; index < _attributes.size(); ++index)
        {
            const Attribute& attribute = _attributes[index];
            std::string_view local = utils::splitQualifiedName(attribute.name).localName;
            if (attribute.name == "xmlns")
                local = "xmlns";
            if (local != localName)
                continue;
            if (attributeNamespace(attribute).value_or("") == namespaceUri)
                return index;
        }
        return npos;
    }

    // Изменяемая версия переиспользует константную (DRY)
    // The mutable version reuses the const one (DRY)
    Element* Element::findChildElementNS(std::string_view namespaceUri, std::string_view localName)
    {
        const Element& self = *this;

        return const_cast<Element*>(self.findChildElementNS(namespaceUri, localName));
    }

    // Первый непосредственный ребёнок с нужными пространством имён и локальным именем
    // The first direct child with the required namespace and local name
    const Element* Element::findChildElementNS(std::string_view namespaceUri,
        std::string_view localName) const
    {
        for (const Node* child = getFirstChild(); child != nullptr; child = child->getNextSibling())
        {
            if (child->getType() != NodeType::ELEMENT)
                continue;
            const Element* element = static_cast<const Element*>(child);
            if (element->getLocalName() != localName)
                continue;
            if (element->getNamespaceURI().value_or("") == namespaceUri)
                return element;
        }
        return nullptr;
    }

    /*
    ** Объявление пространства имён, заданное через setAttributeNS. Раньше
    ** setAttributeNS(XMLNS, "xmlns:p", "") записывало объявление с пустым URI
    ** в обход проверки declareNamespace, а пара (XMLNS, "a:b") связывала
    ** префикс a со служебным пространством xmlns.
    **
    ** A namespace declaration set through setAttributeNS. Previously
    ** setAttributeNS(XMLNS, "xmlns:p", "") wrote a declaration with an empty URI
    ** bypassing the declareNamespace check, and the pair (XMLNS, "a:b") bound
    ** the prefix a to the reserved xmlns namespace.
    */
    void Element::setNamespaceDeclarationNS(std::string_view namespaceUri, std::string_view prefix,
        std::string_view localName, std::string value)
    {
        bool declaration = prefix == "xmlns" || (prefix.empty() && localName == "xmlns");

        if (!declaration || namespaceUri != XMLNS_NAMESPACE_URI)
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                i18n::format(i18n::Message::XMLNS_ATTRIBUTE_NAMESPACE));
        if (prefix.empty())
            declareNamespace("", std::move(value));
        else
            declareNamespace(localName, std::move(value));
    }
}
