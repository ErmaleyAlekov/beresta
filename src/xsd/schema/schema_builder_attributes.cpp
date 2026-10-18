#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "utils/qualified_name.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** Атрибуты типа: xs:attribute, ссылки xs:attributeGroup (раскрываются
    ** рекурсивно) и xs:anyAttribute (допускает любые другие атрибуты).
    **
    ** The attributes of a type: xs:attribute, xs:attributeGroup references (expanded
    ** recursively) and xs:anyAttribute (allows any other attributes).
    */
    void SchemaBuilder::buildAttributes(const Element& parent, const SchemaSource& source,
        ComplexType& owner, std::size_t depth)
    {
        if (depth > 32)
            fail(parent, i18n::format(i18n::Message::SCHEMA_ATTRIBUTE_GROUPS_TOO_DEEP));
        for (const Element* child : schemaChildren(parent))
        {
            std::string_view kind = child->getLocalName();
            if (kind == "attribute")
                addAttribute(owner, buildAttributeUse(*child, source, false));
            else if (kind == "anyAttribute")
            {
                owner.anyAttribute = true;
                owner.attributeWildcard = parseProcessContents(*child);
            }
            else if (kind == "attributeGroup")
            {
                std::string name = attributeOr(*child, "ref", "");
                const Definition* group = findDefinition("attributeGroup", resolveQName(*child, name));
                if (group == nullptr)
                    fail(*child, i18n::format(i18n::Message::SCHEMA_ATTRIBUTE_GROUP_MISSING, {name}));
                buildAttributes(*group->element, *group->source, owner, depth + 1);
            }
        }
    }

    /*
    ** Использование атрибута: имя и пространство имён, тип (атрибут type,
    ** анонимный xs:simpleType или anySimpleType), use, default и fixed.
    ** Для ref тип и ограничения берутся из глобального объявления.
    **
    ** An attribute use: the name and the namespace, the type (the type attribute,
    ** an anonymous xs:simpleType or anySimpleType), use, default and fixed.
    ** For ref the type and the constraints are taken from the global declaration.
    */
    AttributeUse SchemaBuilder::buildAttributeUse(const Element& definition, const SchemaSource& source,
        bool global)
    {
        AttributeUse use;
        const SchemaSource* typeSource = &source;

        use.name = attributeOr(definition, "name", "");
        use.namespaceUri = localNamespace(definition, source, true);
        if (global)
            use.namespaceUri = source.targetNamespace;
        const Element& typed = resolveAttributeReference(definition, use, typeSource);
        use.type = builtinType("anySimpleType");
        if (typed.hasAttribute("type") || firstSchemaChild(typed, "simpleType") != nullptr)
            use.type = simpleTypeReference(typed, "type", *typeSource);
        std::string usage = attributeOr(definition, "use", "optional");
        use.required = usage == "required";
        use.prohibited = usage == "prohibited";
        if (&typed != &definition)
            readValueConstraints(typed, use.defaultValue, use.fixedValue);
        readValueConstraints(definition, use.defaultValue, use.fixedValue);
        return use;
    }

    // ref="p:name" -> глобальный атрибут (или встроенный атрибут пространства xml:)
    // ref="p:name" -> a global attribute (or a built-in attribute of the xml: namespace)
    const Element& SchemaBuilder::resolveAttributeReference(const Element& definition, AttributeUse& use,
        const SchemaSource*& typeSource)
    {
        std::optional<std::string> reference = definition.getAttribute("ref");

        if (!reference.has_value())
            return definition;
        QualifiedKey key = resolveQName(definition, reference.value());
        const Definition* target = findDefinition("attribute", key);
        if (target == nullptr && key.first != XML_NAMESPACE_URI)
            fail(definition, i18n::format(i18n::Message::SCHEMA_ATTRIBUTE_UNDECLARED, {reference.value()}));
        use.name = key.second;
        use.namespaceUri = key.first;
        if (target == nullptr)
            return definition;
        typeSource = target->source;
        return *target->element;
    }

    // Атрибут с тем же именем (например, унаследованный) заменяется новым использованием
    // An attribute with the same name (for example, an inherited one) is replaced with the new use
    void SchemaBuilder::addAttribute(ComplexType& owner, AttributeUse use) const
    {
        for (AttributeUse& existing : owner.attributes)
        {
            if (existing.name == use.name && existing.namespaceUri == use.namespaceUri)
            {
                existing = std::move(use);
                return;
            }
        }
        owner.attributes.push_back(std::move(use));
    }
}
