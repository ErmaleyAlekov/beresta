#include "xsd/internal/schema_builder.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Прежнее определение для ссылки изнутри компонента xs:redefine: от
    ** места ссылки поднимаемся к предкам, пока не встретим сам компонент.
    ** Прежнее возвращается, только если ссылка ведёт на имя этого же
    ** компонента и того же вида; без xs:redefine в схеме — сразу nullptr.
    **
    ** The former definition for a reference from inside an xs:redefine component: from
    ** the reference place we climb to the ancestors until we meet the component itself.
    ** The former one is returned only if the reference leads to the name of this very
    ** component and of the same kind; without xs:redefine in the schema — nullptr at once.
    */
    const Definition* SchemaBuilder::originalFor(const Element& reference, std::string_view kind,
        const QualifiedKey& key) const
    {
        const Node* node = &reference;

        if (_redefinitions.empty())
            return nullptr;
        while (node != nullptr && node->getType() == NodeType::ELEMENT)
        {
            auto found = _redefinitions.find(static_cast<const Element*>(node));
            if (found != _redefinitions.end())
            {
                if (found->second.kind == kind && found->second.key == key)
                    return &found->second.original;
                return nullptr;
            }
            node = node->getParent();
        }
        return nullptr;
    }

    // Определение группы или группы атрибутов по ссылке ref (с учётом xs:redefine)
    // The definition of a group or an attribute group by a ref reference (taking xs:redefine into account)
    const Definition* SchemaBuilder::referencedDefinition(const Element& reference, std::string_view kind,
        const QualifiedKey& key)
    {
        const Definition* original = originalFor(reference, kind, key);

        if (original != nullptr)
            return original;
        return findDefinition(kind, key);
    }

    /*
    ** Тип по ссылке base / itemType / type. Внутри переопределяемого типа
    ** ссылка на собственное имя строит прежний тип — один раз, под его
    ** именем, но без регистрации (имя в схеме принадлежит новому типу).
    **
    ** A type by a base / itemType / type reference. Inside a redefined type
    ** a reference to its own name builds the former type — once, under its
    ** name, but without registration (the name in the schema belongs to the new type).
    */
    TypeRef SchemaBuilder::referencedType(const Element& context, std::string_view value)
    {
        QualifiedKey key = resolveQName(context, value);
        const Definition* original = originalFor(context, "complexType", key);

        if (original == nullptr)
            original = originalFor(context, "simpleType", key);
        if (original == nullptr)
            return resolveType(key);
        auto built = _builtOriginals.find(original->element);
        if (built != _builtOriginals.end())
            return built->second;
        TypeRef type;
        if (isSchemaElement(*original->element, "complexType"))
            type.complex = buildComplexType(*original->element, *original->source, key, false);
        else
            type.simple = buildSimpleType(*original->element, *original->source, key, false);
        _builtOriginals[original->element] = type;
        return type;
    }
}
