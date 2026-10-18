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
        return buildElement(*definition->element, *definition->source, true);
    }

    /*
    ** Объявление элемента. Глобальное регистрируется ДО построения типа:
    ** рекурсивные модели (элемент внутри элемента своего же типа) получают
    ** указатель на уже существующее объявление.
    **
    ** An element declaration. A global one is registered BEFORE its type is built:
    ** recursive models (an element inside an element of its own type) get
    ** a pointer to an already existing declaration.
    */
    ElementDecl* SchemaBuilder::buildElement(const Element& definition, const SchemaSource& source,
        bool global)
    {
        _schema._elements.emplace_back();
        ElementDecl& declaration = _schema._elements.back();

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
        declaration.nillable = attributeOr(definition, "nillable", "false") == "true";
        declaration.isAbstract = attributeOr(definition, "abstract", "false") == "true";
        readValueConstraints(definition, declaration.defaultValue, declaration.fixedValue);
        declaration.type = elementType(definition, source);
        readIdentityConstraints(definition, source, declaration);
        return &declaration;
    }

    /*
    ** Тип элемента: атрибут type, анонимный xs:simpleType / xs:complexType,
    ** тип головы группы замены (substitutionGroup) или xs:anyType.
    **
    ** The element type: the type attribute, an anonymous xs:simpleType / xs:complexType,
    ** the type of the substitution group head (substitutionGroup) or xs:anyType.
    */
    TypeRef SchemaBuilder::elementType(const Element& definition, const SchemaSource& source)
    {
        std::optional<std::string> typeName = definition.getAttribute("type");
        const Element* simple = firstSchemaChild(definition, "simpleType");
        const Element* complex = firstSchemaChild(definition, "complexType");
        std::optional<std::string> head = definition.getAttribute("substitutionGroup");

        if (typeName.has_value())
            return resolveType(resolveQName(definition, typeName.value()));
        if (simple != nullptr)
            return TypeRef{buildSimpleType(*simple, source, QualifiedKey()), nullptr};
        if (complex != nullptr)
            return TypeRef{nullptr, buildComplexType(*complex, source, QualifiedKey())};
        if (head.has_value())
            return globalElement(resolveQName(definition, head.value()))->type;
        return TypeRef();
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
