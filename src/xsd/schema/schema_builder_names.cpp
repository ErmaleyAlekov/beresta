#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Значение типа QName ("xs:string", "tns:Book") разрешается по объявлениям
    ** xmlns в самом документе схемы. Имя без префикса берёт пространство
    ** по умолчанию (xmlns="..."), а при его отсутствии — «без пространства».
    **
    ** A QName value ("xs:string", "tns:Book") is resolved by the xmlns
    ** declarations in the schema document itself. A name without a prefix takes the default
    ** namespace (xmlns="..."), and if there is none — "no namespace".
    ** In a "chameleon" schema a name outside namespaces takes the namespace
    ** of the including schema — the same one its components were registered under.
    */
    QualifiedKey SchemaBuilder::resolveQName(const Element& context, std::string_view value) const
    {
        std::string collapsed = normalizeWhiteSpace(value, WhiteSpace::COLLAPSE);
        utils::QualifiedNameParts parts = utils::splitQualifiedName(collapsed);
        std::optional<std::string> uri = context.lookupNamespaceURI(parts.prefix);

        if (!parts.prefix.empty() && !uri.has_value())
            fail(context, i18n::format(i18n::Message::SCHEMA_PREFIX_UNDECLARED, {parts.prefix, collapsed}));
        std::string namespaceUri = uri.value_or("");
        if (namespaceUri.empty())
            namespaceUri = chameleonNamespace(context);
        return QualifiedKey(namespaceUri, std::string(parts.localName));
    }

    /*
    ** Пространство «хамелеона»: корень документа схемы без targetNamespace,
    ** подключённого через include / redefine / override, получил
    ** пространство включающей схемы. У обычных документов — пустая строка.
    **
    ** The namespace of a "chameleon": the root of a schema document without targetNamespace
    ** included through include / redefine / override got the namespace of the
    ** including schema. For ordinary documents — an empty string.
    */
    std::string SchemaBuilder::chameleonNamespace(const Element& context) const
    {
        const Node* root = &context;

        while (root->getParent() != nullptr && root->getParent()->getType() == NodeType::ELEMENT)
            root = root->getParent();
        for (const SchemaSource& source : _sources)
        {
            if (source.root == root && !source.root->hasAttribute("targetNamespace"))
                return source.targetNamespace;
        }
        return std::string();
    }

    /*
    ** Тип по имени: встроенные типы XSD, уже построенные, либо построение
    ** по зарегистрированному определению (с запоминанием результата).
    **
    ** A type by name: the built-in XSD types, already built ones, or building
    ** from a registered definition (with memoization of the result).
    */
    TypeRef SchemaBuilder::resolveType(const QualifiedKey& key)
    {
        if (key.first == XSD_NAMESPACE_URI)
        {
            if (key.second == "anyType")
                return TypeRef();
            const SimpleType* builtin = builtinType(key.second);
            if (builtin == nullptr)
                fail(*_sources.front().root, i18n::format(i18n::Message::SCHEMA_UNKNOWN_BUILTIN, {key.second}));
            return TypeRef{builtin, nullptr};
        }
        auto built = _builtTypes.find(key);
        if (built != _builtTypes.end())
            return built->second;
        const Definition* complex = findDefinition("complexType", key);
        if (complex != nullptr)
            return TypeRef{nullptr, buildComplexType(*complex->element, *complex->source, key)};
        const Definition* simple = findDefinition("simpleType", key);
        if (simple == nullptr)
            fail(*_sources.front().root, i18n::format(i18n::Message::SCHEMA_TYPE_NOT_FOUND, {key.first, key.second}));
        return TypeRef{buildSimpleType(*simple->element, *simple->source, key), nullptr};
    }

    // Пространство локального элемента или атрибута: form, иначе elementFormDefault / attributeFormDefault
    // The namespace of a local element or attribute: form, otherwise elementFormDefault / attributeFormDefault
    std::string SchemaBuilder::localNamespace(const Element& definition, const SchemaSource& source,
        bool attribute) const
    {
        bool qualified = source.qualifiedElements;

        if (attribute)
            qualified = source.qualifiedAttributes;
        std::string form = attributeOr(definition, "form", "");
        if (!form.empty())
            qualified = form == "qualified";
        if (qualified)
            return source.targetNamespace;
        return std::string();
    }

    // Ошибка схемы с позицией определения (если документ схемы разобран с позициями)
    // A schema error with the definition position (if the schema document was parsed with positions)
    void SchemaBuilder::fail(const Element& where, const std::string& message) const
    {
        const SourceLocation& location = where.getSourceLocation();
        std::string prefix;

        if (location.line > 0)
            prefix = i18n::format(i18n::Message::SCHEMA_AT_LINE, {std::to_string(location.line)});
        throw DomException(DomErrorCode::SCHEMA_ERROR, prefix + message);
    }
}
