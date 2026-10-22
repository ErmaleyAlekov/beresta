#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd::detail
{
    /*
    ** xs:simpleType: restriction (уточнение базового типа фасетами),
    ** list (список значений) или union (значение любого из типов).
    ** Именованный тип регистрируется до построения, как и составной, и
    ** так же помечается незавершённым: ссылка на него из его же тела —
    ** цикл. publish = false — прежнее определение из xs:redefine.
    **
    ** xs:simpleType: restriction (refining the base type with facets),
    ** list (a list of values) or union (a value of any of the types).
    ** A named type is registered before it is built, like a complex one, and
    ** is likewise marked incomplete: a reference to it from its own body is
    ** a cycle. publish = false — the former definition from xs:redefine.
    */
    const SimpleType* SchemaBuilder::buildSimpleType(const Element& definition,
        const SchemaSource& source, const QualifiedKey& name, bool publish)
    {
        NestingGuard nesting(*this, definition);
        _schema._simpleTypes.emplace_back();
        SimpleType& type = _schema._simpleTypes.back();

        type.name = name.second;
        type.namespaceUri = name.first;
        if (type.name.empty())
            type.name = i18n::format(i18n::Message::ANONYMOUS_TYPE);
        if (publish && !name.second.empty())
        {
            _builtTypes[name] = TypeRef{&type, nullptr};
            _schema._globalTypes[name] = TypeRef{&type, nullptr};
        }
        _incomplete.insert(&type);
        buildSimpleBody(type, definition, source);
        _incomplete.erase(&type);
        type.finalSet = derivationAttribute(definition, "final", source.finalDefault, SIMPLE_FINAL_ALL);
        return &type;
    }

    /*
    ** Тело простого типа. Базовый тип list и union — anySimpleType;
    ** у ограничения его задаёт buildRestriction. Тип элементов списка не
    ** должен запрещать list атрибутом final.
    **
    ** The body of a simple type. The base type of list and union is anySimpleType;
    ** for a restriction it is set by buildRestriction. The list item type must
    ** not prohibit list with the final attribute.
    */
    void SchemaBuilder::buildSimpleBody(SimpleType& type, const Element& definition, const SchemaSource& source)
    {
        const Element* restriction = firstSchemaChild(definition, "restriction");
        const Element* list = firstSchemaChild(definition, "list");

        type.baseType = builtinType("anySimpleType");
        if (restriction != nullptr)
            buildRestriction(type, *restriction, source);
        else if (list != nullptr)
        {
            type.variety = Variety::LIST;
            type.itemType = simpleTypeReference(*list, "itemType", source);
            type.facets.whiteSpace = WhiteSpace::COLLAPSE;
            checkFinal(*list, type.itemType->finalSet, type.itemType->name, DERIVATION_LIST);
        }
        else
            buildMembers(type, definition, source);
    }

    /*
    ** Простой тип по атрибуту (base, itemType) или из вложенного анонимного
    ** xs:simpleType. Незавершённый тип — это ссылка тела типа на самого
    ** себя (прямо или через другие типы): циклическое определение.
    **
    ** A simple type by an attribute (base, itemType) or from a nested anonymous
    ** xs:simpleType. An incomplete type is a reference of a type body to itself
    ** (directly or through other types): a circular definition.
    */
    const SimpleType* SchemaBuilder::simpleTypeReference(const Element& owner, std::string_view attribute,
        const SchemaSource& source)
    {
        std::optional<std::string> name = owner.getAttribute(attribute);
        const Element* anonymous = firstSchemaChild(owner, "simpleType");

        if (name.has_value())
        {
            TypeRef type = referencedType(owner, name.value());
            if (type.simple == nullptr)
                fail(owner, i18n::format(i18n::Message::SCHEMA_NOT_SIMPLE_TYPE, {name.value()}));
            if (_incomplete.count(type.simple) != 0)
                fail(owner, i18n::format(i18n::Message::SCHEMA_CYCLIC_DERIVATION));
            return type.simple;
        }
        if (anonymous == nullptr)
            fail(owner, i18n::format(i18n::Message::SCHEMA_NO_SIMPLE_TYPE, {attribute}));
        return buildSimpleType(*anonymous, source, QualifiedKey());
    }

    // Ограничение: копия базового типа (вместе с его фасетами) + собственные фасеты; final базы проверяется
    // A restriction: a copy of the base type (together with its facets) + its own facets; the base's final is checked
    void SchemaBuilder::buildRestriction(SimpleType& type, const Element& restriction,
        const SchemaSource& source)
    {
        const SimpleType* base = simpleTypeReference(restriction, "base", source);
        std::string name = type.name;
        std::string namespaceUri = type.namespaceUri;

        checkFinal(restriction, base->finalSet, base->name, DERIVATION_RESTRICTION);
        type = *base;
        type.name = name;
        type.namespaceUri = namespaceUri;
        type.baseType = base;
        type.finalSet = 0;
        applyFacets(type, restriction, source);
    }

    // Объединение: типы из memberTypes (без циклов и запретов final) и вложенные анонимные типы
    // A union: the types from memberTypes (without cycles and final prohibitions) and the nested anonymous types
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
            if (_incomplete.count(resolved.simple) != 0)
                fail(*unionElement, i18n::format(i18n::Message::SCHEMA_CYCLIC_DERIVATION));
            checkFinal(*unionElement, resolved.simple->finalSet, resolved.simple->name, DERIVATION_UNION);
            type.memberTypes.push_back(resolved.simple);
        }
        for (const Element* child : schemaChildren(*unionElement))
            type.memberTypes.push_back(buildSimpleType(*child, source, QualifiedKey()));
        if (type.memberTypes.empty())
            fail(*unionElement, i18n::format(i18n::Message::SCHEMA_UNION_EMPTY));
    }
}
