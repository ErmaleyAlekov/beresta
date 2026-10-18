#include "parser/parse_session.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/qualified_name.hpp"

#include <unordered_map>
#include <utility>
namespace beresta::detail
{
    /*
    ** Повтор расширенного имени (URI + локальное имя) у тега с множеством
    ** атрибутов: хеш-таблица вместо перебора пар, чтобы не было O(n²).
    ** Ключ — URI и имя через символ \0: в тексте XML его быть не может,
    ** поэтому разные пары не склеятся в один ключ.
    **
    ** A repeated expanded name (URI + local name) in a tag with many
    ** attributes: a hash table instead of trying pairs, so that there is no O(n²).
    ** The key is the URI and the name joined by the \0 character: it cannot occur in XML text,
    ** so different pairs do not merge into one key.
    */
    static void checkManyExpandedNames(const std::vector<SaxAttribute>& attributes)
    {
        std::unordered_map<std::string, std::size_t, utils::SeededHash> seen;

        for (std::size_t index = 0; index < attributes.size(); ++index)
        {
            const SaxAttribute& attribute = attributes[index];
            if (attribute.namespaceUri.empty())
                continue;
            std::string key(attribute.namespaceUri);
            key += '\0';
            key.append(attribute.localName);
            auto inserted = seen.emplace(std::move(key), index);
            if (!inserted.second)
                throw DomException(DomErrorCode::NAMESPACE_ERROR, i18n::format(i18n::Message::ATTRIBUTES_SAME_NAME,
                    {attributes[inserted.first->second].name, attribute.name}));
        }
    }

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
        if (_saxAttributes.size() > utils::LINEAR_SEARCH_LIMIT)
        {
            checkManyExpandedNames(_saxAttributes);
            return;
        }
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
