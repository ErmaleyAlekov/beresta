#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd::detail
{
    /*
    ** xs:simpleType: restriction (уточнение базового типа фасетами),
    ** list (список значений) или union (значение любого из типов).
    ** Именованный тип регистрируется до построения, как и составной.
    **
    ** xs:simpleType: restriction (refining the base type with facets),
    ** list (a list of values) or union (a value of any of the types).
    ** A named type is registered before it is built, like a complex one.
    */
    const SimpleType* SchemaBuilder::buildSimpleType(const Element& definition,
        const SchemaSource& source, const QualifiedKey& name)
    {
        _schema._simpleTypes.emplace_back();
        SimpleType& type = _schema._simpleTypes.back();
        const Element* restriction = firstSchemaChild(definition, "restriction");
        const Element* list = firstSchemaChild(definition, "list");

        type.name = name.second;
        type.namespaceUri = name.first;
        if (type.name.empty())
            type.name = i18n::format(i18n::Message::ANONYMOUS_TYPE);
        if (!name.second.empty())
        {
            _builtTypes[name] = TypeRef{&type, nullptr};
            _schema._globalTypes[name] = TypeRef{&type, nullptr};
        }
        if (restriction != nullptr)
            buildRestriction(type, *restriction, source);
        else if (list != nullptr)
        {
            type.variety = Variety::LIST;
            type.itemType = simpleTypeReference(*list, "itemType", source);
            type.facets.whiteSpace = WhiteSpace::COLLAPSE;
        }
        else
            buildMembers(type, definition, source);
        return &type;
    }

    // Простой тип по атрибуту (base, itemType) или из вложенного анонимного xs:simpleType
    // A simple type by an attribute (base, itemType) or from a nested anonymous xs:simpleType
    const SimpleType* SchemaBuilder::simpleTypeReference(const Element& owner, std::string_view attribute,
        const SchemaSource& source)
    {
        std::optional<std::string> name = owner.getAttribute(attribute);
        const Element* anonymous = firstSchemaChild(owner, "simpleType");

        if (name.has_value())
        {
            TypeRef type = resolveType(resolveQName(owner, name.value()));
            if (type.simple == nullptr)
                fail(owner, i18n::format(i18n::Message::SCHEMA_NOT_SIMPLE_TYPE, {name.value()}));
            return type.simple;
        }
        if (anonymous == nullptr)
            fail(owner, i18n::format(i18n::Message::SCHEMA_NO_SIMPLE_TYPE, {attribute}));
        return buildSimpleType(*anonymous, source, QualifiedKey());
    }

    // Ограничение: копия базового типа (вместе с его фасетами) + собственные фасеты
    // A restriction: a copy of the base type (together with its facets) + its own facets
    void SchemaBuilder::buildRestriction(SimpleType& type, const Element& restriction,
        const SchemaSource& source)
    {
        const SimpleType* base = simpleTypeReference(restriction, "base", source);
        std::string name = type.name;
        std::string namespaceUri = type.namespaceUri;

        type = *base;
        type.name = name;
        type.namespaceUri = namespaceUri;
        applyFacets(type, restriction);
    }

    // Объединение: типы из memberTypes и вложенные анонимные типы
    // A union: the types from memberTypes and the nested anonymous types
    void SchemaBuilder::buildMembers(SimpleType& type, const Element& definition,
        const SchemaSource& source)
    {
        const Element* unionElement = firstSchemaChild(definition, "union");

        if (unionElement == nullptr)
            fail(definition, i18n::format(i18n::Message::SCHEMA_SIMPLE_TYPE_BODY));
        type.variety = Variety::UNION;
        std::string members = attributeOr(*unionElement, "memberTypes", "");
        for (std::string_view member : utils::splitTokens(members))
        {
            TypeRef resolved = resolveType(resolveQName(*unionElement, member));
            if (resolved.simple == nullptr)
                fail(*unionElement, i18n::format(i18n::Message::SCHEMA_UNION_MEMBER));
            type.memberTypes.push_back(resolved.simple);
        }
        for (const Element* child : schemaChildren(*unionElement))
            type.memberTypes.push_back(buildSimpleType(*child, source, QualifiedKey()));
        if (type.memberTypes.empty())
            fail(*unionElement, i18n::format(i18n::Message::SCHEMA_UNION_EMPTY));
    }
}
