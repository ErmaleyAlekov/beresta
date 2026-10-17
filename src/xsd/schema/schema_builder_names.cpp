#include "xsd/internal/schema_builder.hpp"
#include "exception/dom_exception.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Значение типа QName ("xs:string", "tns:Book") разрешается по объявлениям
    ** xmlns в самом документе схемы. Имя без префикса берёт пространство
    ** по умолчанию (xmlns="..."), а при его отсутствии — «без пространства».
    */
    QualifiedKey SchemaBuilder::resolveQName(const Element& context, std::string_view value) const
    {
        std::string collapsed = normalizeWhiteSpace(value, WhiteSpace::COLLAPSE);
        utils::QualifiedNameParts parts = utils::splitQualifiedName(collapsed);
        std::optional<std::string> uri = context.lookupNamespaceURI(parts.prefix);

        if (!parts.prefix.empty() && !uri.has_value())
            fail(context, "префикс \"" + std::string(parts.prefix) + "\" в \"" + collapsed + "\" не объявлен");
        return QualifiedKey(uri.value_or(""), std::string(parts.localName));
    }

    /*
    ** Тип по имени: встроенные типы XSD, уже построенные, либо построение
    ** по зарегистрированному определению (с запоминанием результата).
    */
    TypeRef SchemaBuilder::resolveType(const QualifiedKey& key)
    {
        if (key.first == XSD_NAMESPACE_URI)
        {
            if (key.second == "anyType")
                return TypeRef();
            const SimpleType* builtin = builtinType(key.second);
            if (builtin == nullptr)
                fail(*_sources.front().root, "неизвестный встроенный тип xs:" + key.second);
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
            fail(*_sources.front().root, "тип {" + key.first + "}" + key.second + " не найден в схеме");
        return TypeRef{buildSimpleType(*simple->element, *simple->source, key), nullptr};
    }

    // Пространство локального элемента или атрибута: form, иначе elementFormDefault / attributeFormDefault
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
    void SchemaBuilder::fail(const Element& where, const std::string& message) const
    {
        const SourceLocation& location = where.getSourceLocation();
        std::string prefix;

        if (location.line > 0)
            prefix = "схема, строка " + std::to_string(location.line) + ": ";
        throw DomException(DomErrorCode::SCHEMA_ERROR, prefix + message);
    }
}
