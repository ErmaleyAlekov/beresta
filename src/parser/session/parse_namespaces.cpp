#include "parser/parse_session.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::detail
{
    /*
    ** Сначала все объявления xmlns этого тега (они действуют на сам тег
    ** и его атрибуты), затем — разрешение префикса элемента и атрибутов.
    ** Необъявленный префикс — фатальная ошибка NAMESPACE_ERROR, как в Xerces.
    **
    ** First all xmlns declarations of this tag (they apply to the tag itself
    ** and its attributes), then resolution of the prefixes of the element and the attributes.
    ** An undeclared prefix is a fatal NAMESPACE_ERROR, as in Xerces.
    */
    void ParseSession::processNamespaces(OpenElement& element,
        const std::vector<Attribute>& attributes)
    {
        _namespaces.pushScope();
        for (const Attribute& attribute : attributes)
        {
            if (utils::isNamespaceDeclaration(attribute.name))
                _namespaces.declare(utils::declaredPrefix(attribute.name), attribute.value);
        }
        if (!utils::hasQualifiedNameShape(element.name))
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                i18n::format(i18n::Message::ELEMENT_QNAME_INVALID, {element.name}));
        utils::QualifiedNameParts parts = utils::splitQualifiedName(element.name);
        if (parts.prefix == "xmlns")
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                i18n::format(i18n::Message::ELEMENT_XMLNS_PREFIX));
        const std::string* uri = _namespaces.resolve(parts.prefix);
        if (!parts.prefix.empty() && uri == nullptr)
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                i18n::format(i18n::Message::PREFIX_UNDECLARED, {parts.prefix}));
        if (uri != nullptr)
            element.namespaceUri = *uri;
        resolveAttributeNamespaces(attributes);
        checkExpandedNames();
    }

    /*
    ** Атрибут без префикса НЕ принадлежит пространству по умолчанию;
    ** объявления xmlns / xmlns:* принадлежат служебному пространству XMLNS.
    **
    ** An attribute without a prefix does NOT belong to the default namespace;
    ** xmlns / xmlns:* declarations belong to the service XMLNS namespace.
    */
    void ParseSession::resolveAttributeNamespaces(const std::vector<Attribute>& attributes)
    {
        _saxAttributes.clear();
        for (const Attribute& attribute : attributes)
        {
            if (!utils::hasQualifiedNameShape(attribute.name))
                throw DomException(DomErrorCode::NAMESPACE_ERROR,
                    i18n::format(i18n::Message::ATTRIBUTE_QNAME_INVALID, {attribute.name}));
            utils::QualifiedNameParts parts = utils::splitQualifiedName(attribute.name);
            std::string_view uri;
            if (utils::isNamespaceDeclaration(attribute.name))
                uri = XMLNS_NAMESPACE_URI;
            else if (!parts.prefix.empty())
            {
                const std::string* bound = _namespaces.resolve(parts.prefix);
                if (bound == nullptr)
                    throw DomException(DomErrorCode::NAMESPACE_ERROR,
                        i18n::format(i18n::Message::PREFIX_UNDECLARED, {parts.prefix}));
                uri = *bound;
            }
            _saxAttributes.push_back(SaxAttribute{attribute.name, attribute.value, uri, parts.localName});
        }
    }

    // Два атрибута с разными префиксами, но одним URI и локальным именем — ошибка
    // Two attributes with different prefixes but the same URI and local name are an error
    void ParseSession::checkExpandedNames() const
    {
        for (std::size_t first = 0; first < _saxAttributes.size(); ++first)
        {
            const SaxAttribute& left = _saxAttributes[first];
            if (left.namespaceUri.empty())
                continue;
            for (std::size_t second = first + 1; second < _saxAttributes.size(); ++second)
            {
                const SaxAttribute& right = _saxAttributes[second];
                if (left.namespaceUri == right.namespaceUri && left.localName == right.localName)
                    throw DomException(DomErrorCode::NAMESPACE_ERROR, i18n::format(i18n::Message::ATTRIBUTES_SAME_NAME, {left.name, right.name}));
            }
        }
    }
}
