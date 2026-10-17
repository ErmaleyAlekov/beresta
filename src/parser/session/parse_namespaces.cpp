#include "parser/parse_session.hpp"
#include "exception/dom_exception.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::detail
{
    /*
    ** Сначала все объявления xmlns этого тега (они действуют на сам тег
    ** и его атрибуты), затем — разрешение префикса элемента и атрибутов.
    ** Необъявленный префикс — фатальная ошибка NAMESPACE_ERROR, как в Xerces.
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
        if (!utils::isValidQualifiedName(element.name))
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                "недопустимое квалифицированное имя элемента \"" + element.name + "\"");
        utils::QualifiedNameParts parts = utils::splitQualifiedName(element.name);
        const std::string* uri = _namespaces.resolve(parts.prefix);
        if (!parts.prefix.empty() && uri == nullptr)
            throw DomException(DomErrorCode::NAMESPACE_ERROR,
                "префикс \"" + std::string(parts.prefix) + "\" не объявлен");
        if (uri != nullptr)
            element.namespaceUri = *uri;
        resolveAttributeNamespaces(attributes);
        checkExpandedNames();
    }

    /*
    ** Атрибут без префикса НЕ принадлежит пространству по умолчанию;
    ** объявления xmlns / xmlns:* принадлежат служебному пространству XMLNS.
    */
    void ParseSession::resolveAttributeNamespaces(const std::vector<Attribute>& attributes)
    {
        _saxAttributes.clear();
        for (const Attribute& attribute : attributes)
        {
            if (!utils::isValidQualifiedName(attribute.name))
                throw DomException(DomErrorCode::NAMESPACE_ERROR,
                    "недопустимое квалифицированное имя атрибута \"" + attribute.name + "\"");
            utils::QualifiedNameParts parts = utils::splitQualifiedName(attribute.name);
            std::string_view uri;
            if (utils::isNamespaceDeclaration(attribute.name))
                uri = XMLNS_NAMESPACE_URI;
            else if (!parts.prefix.empty())
            {
                const std::string* bound = _namespaces.resolve(parts.prefix);
                if (bound == nullptr)
                    throw DomException(DomErrorCode::NAMESPACE_ERROR,
                        "префикс \"" + std::string(parts.prefix) + "\" не объявлен");
                uri = *bound;
            }
            _saxAttributes.push_back(SaxAttribute{attribute.name, attribute.value, uri, parts.localName});
        }
    }

    // Два атрибута с разными префиксами, но одним URI и локальным именем — ошибка
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
                    throw DomException(DomErrorCode::NAMESPACE_ERROR, "атрибуты \""
                        + std::string(left.name) + "\" и \"" + std::string(right.name)
                        + "\" обозначают одно и то же имя");
            }
        }
    }
}
