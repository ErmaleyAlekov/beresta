#include "xsd/internal/schema_builder.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd::detail
{
    /*
    ** xs:simpleType: restriction (уточнение базового типа фасетами),
    ** list (список значений) или union (значение любого из типов).
    ** Именованный тип регистрируется до построения, как и составной.
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
            type.name = "(анонимный тип)";
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
    const SimpleType* SchemaBuilder::simpleTypeReference(const Element& owner, std::string_view attribute,
        const SchemaSource& source)
    {
        std::optional<std::string> name = owner.getAttribute(attribute);
        const Element* anonymous = firstSchemaChild(owner, "simpleType");

        if (name.has_value())
        {
            TypeRef type = resolveType(resolveQName(owner, name.value()));
            if (type.simple == nullptr)
                fail(owner, "\"" + name.value() + "\" не является простым типом");
            return type.simple;
        }
        if (anonymous == nullptr)
            fail(owner, "не указан ни атрибут " + std::string(attribute) + ", ни вложенный xs:simpleType");
        return buildSimpleType(*anonymous, source, QualifiedKey());
    }

    // Ограничение: копия базового типа (вместе с его фасетами) + собственные фасеты
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
    void SchemaBuilder::buildMembers(SimpleType& type, const Element& definition,
        const SchemaSource& source)
    {
        const Element* unionElement = firstSchemaChild(definition, "union");

        if (unionElement == nullptr)
            fail(definition, "xs:simpleType должен содержать restriction, list или union");
        type.variety = Variety::UNION;
        std::string members = attributeOr(*unionElement, "memberTypes", "");
        for (std::string_view member : utils::splitTokens(members))
        {
            TypeRef resolved = resolveType(resolveQName(*unionElement, member));
            if (resolved.simple == nullptr)
                fail(*unionElement, "член объединения должен быть простым типом");
            type.memberTypes.push_back(resolved.simple);
        }
        for (const Element* child : schemaChildren(*unionElement))
            type.memberTypes.push_back(buildSimpleType(*child, source, QualifiedKey()));
        if (type.memberTypes.empty())
            fail(*unionElement, "объединение без типов-членов");
    }
}
