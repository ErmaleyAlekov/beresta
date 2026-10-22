#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    // Глобальный элемент строится один раз; ссылки ref указывают на одно объявление
    // A global element is built once; ref references point to one declaration
    const ElementDecl* SchemaBuilder::globalElement(const QualifiedKey& key)
    {
        auto built = _builtElements.find(key);

        if (built != _builtElements.end())
            return built->second;
        const Definition* definition = findDefinition("element", key);
        if (definition == nullptr)
            fail(*_sources.front().root, i18n::format(i18n::Message::SCHEMA_ELEMENT_UNDECLARED, {key.first, key.second}));
        checkElementAttributes(*definition->element, ElementPlace::GLOBAL_ELEMENT);
        return buildElement(*definition->element, *definition->source, true);
    }

    /*
    ** Объявление элемента. Регистрируется ДО построения типа: рекурсивные
    ** модели получают указатель на уже существующее объявление. Глобальное —
    ** по имени, локальное — по узлу XML: локальный элемент именованной
    ** группы — один компонент, сколько бы раз группа ни подставлялась.
    ** Иначе группа, чей локальный элемент имеет анонимный тип со ссылкой
    ** на ту же группу, разворачивалась бы бесконечно.
    **
    ** An element declaration. It is registered BEFORE its type is built: recursive
    ** models get a pointer to an already existing declaration. A global one is
    ** registered by name, a local one by the XML node: a local element of a named
    ** group is one component, however many times the group is substituted.
    ** Otherwise a group whose local element has an anonymous type referring
    ** to the same group would be expanded endlessly.
    */
    ElementDecl* SchemaBuilder::buildElement(const Element& definition, const SchemaSource& source,
        bool global)
    {
        NestingGuard nesting(*this, definition);
        _schema._elements.emplace_back();
        ElementDecl& declaration = _schema._elements.back();
        std::optional<std::string> head = definition.getAttribute("substitutionGroup");

        declaration.name = attributeOr(definition, "name", "");
        if (declaration.name.empty())
            fail(definition, i18n::format(i18n::Message::SCHEMA_ELEMENT_NO_NAME));
        declaration.namespaceUri = localNamespace(definition, source, false);
        if (global)
        {
            declaration.namespaceUri = source.targetNamespace;
            QualifiedKey key(declaration.namespaceUri, declaration.name);
            _builtElements[key] = &declaration;
            _schema._globalElements[key] = &declaration;
        }
        else
            _builtLocals[&definition] = &declaration;
        readElementFlags(definition, source, declaration);
        readValueConstraints(definition, declaration.defaultValue, declaration.fixedValue);
        if (global && head.has_value())
            declaration.substitutionHead = globalElement(resolveQName(definition, head.value()));
        declaration.type = elementType(definition, source, declaration);
        readIdentityConstraints(definition, source, declaration);
        return &declaration;
    }

    // nillable, abstract и запреты block / final (по умолчанию — blockDefault / finalDefault схемы)
    // nillable, abstract and the block / final prohibitions (by default — the schema's blockDefault / finalDefault)
    void SchemaBuilder::readElementFlags(const Element& definition, const SchemaSource& source,
        ElementDecl& declaration) const
    {
        declaration.nillable = booleanAttribute(definition, "nillable", false);
        declaration.isAbstract = booleanAttribute(definition, "abstract", false);
        declaration.blockSet = derivationAttribute(definition, "block", source.blockDefault, ELEMENT_BLOCK_ALL);
        declaration.finalSet = derivationAttribute(definition, "final", source.finalDefault,
            COMPLEX_DERIVATIONS_ALL);
    }

    /*
    ** Тип элемента: атрибут type, анонимный xs:simpleType / xs:complexType,
    ** тип головы группы замены (substitutionGroup) или xs:anyType. Голова
    ** может ещё строиться (её тип ссылается на этот элемент), поэтому тип,
    ** взятый у головы, уточняется после загрузки схемы (inheritHeadTypes).
    **
    ** The element type: the type attribute, an anonymous xs:simpleType / xs:complexType,
    ** the type of the substitution group head (substitutionGroup) or xs:anyType. The head
    ** may still be under construction (its type refers to this element), so the type
    ** taken from the head is refined after the schema is loaded (inheritHeadTypes).
    */
    TypeRef SchemaBuilder::elementType(const Element& definition, const SchemaSource& source,
        ElementDecl& declaration)
    {
        std::optional<std::string> typeName = definition.getAttribute("type");
        const Element* simple = firstSchemaChild(definition, "simpleType");
        const Element* complex = firstSchemaChild(definition, "complexType");

        if (typeName.has_value())
            return resolveType(resolveQName(definition, typeName.value()));
        if (simple != nullptr)
            return TypeRef{buildSimpleType(*simple, source, QualifiedKey()), nullptr};
        if (complex != nullptr)
            return TypeRef{nullptr, buildComplexType(*complex, source, QualifiedKey())};
        if (declaration.substitutionHead == nullptr)
            return TypeRef();
        _headTyped.insert(&declaration);
        return declaration.substitutionHead->type;
    }

    // Ограничения значения: default и fixed (взаимоисключающие по спецификации)
    // Value constraints: default and fixed (mutually exclusive by the specification)
    void SchemaBuilder::readValueConstraints(const Element& definition,
        std::optional<std::string>& defaultValue, std::optional<std::string>& fixedValue) const
    {
        std::optional<std::string> byDefault = definition.getAttribute("default");
        std::optional<std::string> byFixed = definition.getAttribute("fixed");

        if (byDefault.has_value() && byFixed.has_value())
            fail(definition, i18n::format(i18n::Message::SCHEMA_DEFAULT_AND_FIXED));
        if (byDefault.has_value())
            defaultValue = byDefault;
        if (byFixed.has_value())
            fixedValue = byFixed;
    }
}
