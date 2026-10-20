#include "xsd/internal/schema_builder.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** xs:simpleContent: текст простого типа + атрибуты. Расширение берёт
    ** тип текста как есть; ограничение создаёт производный простой тип
    ** с фасетами из xs:restriction. Атрибуты базового типа наследуются.
    ** Свои утверждения xs:assert идут после унаследованных (completeBase).
    **
    ** xs:simpleContent: text of a simple type + attributes. An extension takes
    ** the text type as is; a restriction creates a derived simple type
    ** with the facets from xs:restriction. The attributes of the base type are inherited.
    ** Own xs:assert assertions follow the inherited ones (completeBase).
    */
    void SchemaBuilder::buildSimpleContent(ComplexType& type, const Element& simpleContent,
        const SchemaSource& source)
    {
        bool extension = false;
        const Element& derivation = derivationOf(simpleContent, extension);
        const ComplexType* complexBase = completeBase(derivation, extension, type);

        type.content = ContentType::SIMPLE_CONTENT;
        type.simpleContent = type.baseType.simple;
        if (complexBase != nullptr)
        {
            type.simpleContent = complexBase->simpleContent;
            type.attributes = complexBase->attributes;
        }
        if (type.simpleContent == nullptr)
            type.simpleContent = builtinType("anySimpleType");
        if (!extension)
        {
            // Тип текста — ограничение прежнего: база запоминается для проверки наследования
            // The text type is a restriction of the former one: the base is remembered for derivation checks
            _schema._simpleTypes.push_back(*type.simpleContent);
            SimpleType& restricted = _schema._simpleTypes.back();
            restricted.baseType = type.simpleContent;
            restricted.finalSet = 0;
            applyFacets(restricted, derivation, source);
            type.simpleContent = &restricted;
        }
        applyAttributeWildcard(type, buildAttributes(derivation, source, type, 0));
        if (extension && complexBase != nullptr)
            inheritAttributeWildcard(type, *complexBase, derivation);
        readAssertions(derivation, source, type);
    }

    /*
    ** xs:complexContent: расширение дописывает собственную модель ПОСЛЕ
    ** модели базового типа и добавляет атрибуты; ограничение задаёт
    ** модель заново, сохраняя базовые атрибуты (если их не переопределили).
    **
    ** xs:complexContent: an extension appends its own model AFTER the
    ** model of the base type and adds attributes; a restriction defines the
    ** model anew, keeping the base attributes (unless they are overridden).
    */
    void SchemaBuilder::buildComplexContent(ComplexType& type, const Element& complexContent,
        const SchemaSource& source, bool mixed)
    {
        bool extension = false;
        const Element& derivation = derivationOf(complexContent, extension);
        const ComplexType* complexBase = completeBase(derivation, extension, type);

        mixed = booleanAttribute(complexContent, "mixed", mixed);
        if (complexBase != nullptr)
        {
            type.attributes = complexBase->attributes;
            if (extension)
                type.elements = complexBase->elements;
        }
        buildTypeBody(type, derivation, source, mixed);
        if (extension && complexBase != nullptr)
            inheritAttributeWildcard(type, *complexBase, derivation);
        if (extension && complexBase != nullptr && complexBase->hasParticle)
            prependBaseParticle(type, *complexBase);
    }

    // Модель расширения = последовательность (модель базового типа, собственная модель)
    // The extension model = a sequence (the base type model, the own model)
    void SchemaBuilder::prependBaseParticle(ComplexType& type, const ComplexType& base)
    {
        content::Particle combined;

        combined.children.push_back(base.particle);
        if (type.hasParticle)
            combined.children.push_back(std::move(type.particle));
        type.particle = std::move(combined);
        type.hasParticle = true;
        if (type.content == ContentType::EMPTY_CONTENT)
            type.content = base.content;
    }
}
