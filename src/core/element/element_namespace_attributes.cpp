#include "core/element.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/qualified_name.hpp"

#include <utility>

namespace beresta
{
    // Объявление: xmlns="uri" для пустого префикса, иначе xmlns:prefix="uri"
    // A declaration: xmlns="uri" for an empty prefix, otherwise xmlns:prefix="uri"
    void Element::declareNamespace(std::string_view prefix, std::string namespaceUri)
    {
        if (prefix == "xml" || prefix == "xmlns")
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                i18n::format(i18n::Message::PREFIX_RESERVED_DECLARE, {prefix}));
        if (prefix.empty())
        {
            setAttribute("xmlns", std::move(namespaceUri));
            return;
        }
        if (namespaceUri.empty())
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                i18n::format(i18n::Message::PREFIX_EMPTY_URI, {prefix}));
        setAttribute("xmlns:" + std::string(prefix), std::move(namespaceUri));
    }

    // Значение атрибута по пространству имён
    // An attribute value by namespace
    std::optional<std::string> Element::getAttributeNS(std::string_view namespaceUri,
        std::string_view localName) const
    {
        std::size_t index = findAttributeNS(namespaceUri, localName);

        if (index == npos)
            return std::nullopt;
        return _attributes[index].value;
    }

    // Наличие атрибута по пространству имён
    // Whether an attribute exists by namespace
    bool Element::hasAttributeNS(std::string_view namespaceUri, std::string_view localName) const
    {
        return findAttributeNS(namespaceUri, localName) != npos;
    }

    // Удаление атрибута по пространству имён
    // Removes an attribute by namespace
    bool Element::removeAttributeNS(std::string_view namespaceUri, std::string_view localName)
    {
        std::size_t index = findAttributeNS(namespaceUri, localName);

        if (index == npos)
            return false;
        _attributes.erase(_attributes.begin() + static_cast<std::ptrdiff_t>(index));
        return true;
    }

    /*
    ** Установка атрибута с пространством имён. Если префикс ещё не связан
    ** с нужным URI в этой точке дерева, объявление xmlns:prefix добавляется
    ** автоматически — получившийся XML всегда корректен.
    **
    ** Sets an attribute with a namespace. If the prefix is not yet bound
    ** to the required URI at this point of the tree, the xmlns:prefix declaration is added
    ** automatically — the resulting XML is always valid.
    */
    void Element::setAttributeNS(std::string_view namespaceUri, std::string_view qualifiedName,
        std::string value)
    {
        utils::QualifiedNameParts parts = utils::splitQualifiedName(qualifiedName);

        if (!utils::isValidQualifiedName(qualifiedName))
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                i18n::format(i18n::Message::QUALIFIED_NAME_INVALID, {qualifiedName}));
        if (parts.prefix.empty() && !namespaceUri.empty())
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                i18n::format(i18n::Message::UNPREFIXED_ATTRIBUTE_NAMESPACE));
        if (!parts.prefix.empty() && lookupNamespaceURI(parts.prefix) != namespaceUri)
            declareNamespace(parts.prefix, std::string(namespaceUri));
        std::size_t index = findAttributeNS(namespaceUri, parts.localName);
        if (index != npos)
            _attributes[index].value = std::move(value);
        else
            setAttribute(qualifiedName, std::move(value));
    }
}
