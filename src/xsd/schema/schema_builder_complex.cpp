#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** xs:complexType. РћР±СЉРµРєС‚ СЂРµРіРёСЃС‚СЂРёСЂСѓРµС‚СЃСЏ РґРѕ РїРѕСЃС‚СЂРѕРµРЅРёСЏ СЃРѕРґРµСЂР¶РёРјРѕРіРѕ
    ** (СЂРµРєСѓСЂСЃРёРІРЅС‹Рµ РјРѕРґРµР»Рё) Рё РїРѕРјРµС‡Р°РµС‚СЃСЏ РЅРµР·Р°РІРµСЂС€С‘РЅРЅС‹Рј. Р•СЃР»Рё Р±Р°Р·РѕРІС‹Р№ С‚РёРї РµС‰С‘
    ** СЃС‚СЂРѕРёС‚СЃСЏ (РµРіРѕ РјРѕРґРµР»СЊ СЃСЃС‹Р»Р°РµС‚СЃСЏ РЅР° СЌР»РµРјРµРЅС‚ СЌС‚РѕРіРѕ С‚РёРїР° вЂ” РїР°С‚С‚РµСЂРЅ Composite),
    ** СЃРѕРґРµСЂР¶РёРјРѕРµ РѕС‚РєР»Р°РґС‹РІР°РµС‚СЃСЏ РґРѕ Р·Р°РІРµСЂС€РµРЅРёСЏ Р±Р°Р·С‹ (schema_builder_pending.cpp):
    ** СЌС‚Рѕ СЂРµРєСѓСЂСЃРёРІРЅР°СЏ СЃС‚СЂСѓРєС‚СѓСЂР° РґР°РЅРЅС‹С…, Р° РЅРµ С†РёРєР» РЅР°СЃР»РµРґРѕРІР°РЅРёСЏ.
    ** publish = false вЂ” РїСЂРµР¶РЅРµРµ РѕРїСЂРµРґРµР»РµРЅРёРµ, Р·Р°РјРµРЅС‘РЅРЅРѕРµ xs:redefine: РѕРЅРѕ
    ** СЃРѕС…СЂР°РЅСЏРµС‚ РёРјСЏ, РЅРѕ РїРѕРґ РЅРёРј РЅРµ СЂРµРіРёСЃС‚СЂРёСЂСѓРµС‚СЃСЏ (РёРјСЏ Р·Р°РЅСЏС‚Рѕ РЅРѕРІС‹Рј).
    **
    ** xs:complexType. The object is registered before its content is built
    ** (recursive models) and marked incomplete. If the base type is still being
    ** built (its model refers to an element of this type вЂ” the Composite pattern),
    ** the content is deferred until the base is complete (schema_builder_pending.cpp):
    ** it is a recursive data structure, not a derivation cycle.
    ** publish = false вЂ” the former definition replaced by xs:redefine: it keeps
    ** its name but is not registered under it (the name belongs to the new one).
    */
    const ComplexType* SchemaBuilder::buildComplexType(const Element& definition,
        const SchemaSource& source, const QualifiedKey& name, bool publish)
    {
        NestingGuard nesting(*this, definition);
        _schema._complexTypes.emplace_back();
        ComplexType& type = _schema._complexTypes.back();
        PendingType job{&type, &definition, &source, nullptr, TypeRef()};

        type.name = name.second;
        type.namespaceUri = name.first;
        readTypeFlags(definition, source, type);
        if (publish && !name.second.empty())
        {
            _builtTypes[name] = TypeRef{nullptr, &type};
            _schema._globalTypes[name] = TypeRef{nullptr, &type};
        }
        _incomplete.insert(&type);
        if (readDerivationBase(job))
            finishComplexType(job);
        else
            _pendingTypes.push_back(job);
        return &type;
    }

    /*
    ** РўРµР»Рѕ С‚РёРїР°: РЅРµРѕР±СЏР·Р°С‚РµР»СЊРЅР°СЏ РјРѕРґРµР»СЊ (sequence / choice / all / group),
    ** Р°С‚СЂРёР±СѓС‚С‹ Рё СѓС‚РІРµСЂР¶РґРµРЅРёСЏ xs:assert. РњРѕРґРµР»СЊ Р·Р°РјРµРЅСЏРµС‚ СѓРЅР°СЃР»РµРґРѕРІР°РЅРЅСѓСЋ С‚РѕР»СЊРєРѕ
    ** РµСЃР»Рё РѕРЅР° РµСЃС‚СЊ; СЃРєР»РµР№РєСѓ РїСЂРё СЂР°СЃС€РёСЂРµРЅРёРё РІС‹РїРѕР»РЅСЏРµС‚ buildComplexContent.
    **
    ** The type body: an optional model (sequence / choice / all / group),
    ** attributes and xs:assert assertions. The model replaces the inherited one only
    ** if it exists; the merge on extension is done by buildComplexContent.
    */
    void SchemaBuilder::buildTypeBody(ComplexType& type, const Element& body, const SchemaSource& source,
        bool mixed)
    {
        for (const Element* child : schemaChildren(body))
        {
            std::string_view kind = child->getLocalName();
            if (kind != "sequence" && kind != "choice" && kind != "all" && kind != "group")
                continue;
            type.particle = buildParticle(*child, source, type, 0);
            const auto particleKind = type.particle.kind;
            const bool emptyGroup = (particleKind == content::ParticleKind::SEQUENCE
                || particleKind == content::ParticleKind::ALL)
                && type.particle.children.empty();
            type.hasParticle = type.particle.maxOccurs != 0 && !emptyGroup;
            break;
        }
        if (type.hasParticle || mixed)
            type.content = ContentType::ELEMENT_ONLY;
        if (mixed)
            type.content = ContentType::MIXED_CONTENT;
        applyAttributeWildcard(type, buildAttributes(body, source, type, 0));
        readAssertions(body, source, type);
    }

    // РќР°СЃР»РµРґРѕРІР°РЅРёРµ Р·Р°РґР°С‘С‚СЃСЏ РµРґРёРЅСЃС‚РІРµРЅРЅС‹Рј xs:extension РёР»Рё xs:restriction
    // Derivation is defined by a single xs:extension or xs:restriction
    const Element& SchemaBuilder::derivationOf(const Element& content, bool& extension) const
    {
        const Element* derivation = firstSchemaChild(content, "extension");

        extension = derivation != nullptr;
        if (derivation == nullptr)
            derivation = firstSchemaChild(content, "restriction");
        if (derivation == nullptr)
            fail(content, i18n::format(i18n::Message::SCHEMA_DERIVATION_EXPECTED));
        return *derivation;
    }

    /*
    ** РЎРїРѕСЃРѕР± РЅР°СЃР»РµРґРѕРІР°РЅРёСЏ Р·Р°РїРѕРјРёРЅР°РµС‚СЃСЏ РІ С‚РёРїРµ, Р±Р°Р·РѕРІС‹Р№ С‚РёРї СѓР¶Рµ Р·Р°РїРёСЃР°РЅ
    ** (readDerivationBase, finishComplexType) вЂ” РїРѕ РЅРёРј РїСЂРѕРІРµСЂСЏРµС‚СЃСЏ xsi:type.
    ** Рљ СЌС‚РѕРјСѓ РјРѕРјРµРЅС‚Сѓ Р±Р°Р·Р° Р·Р°РІРµСЂС€РµРЅР°: С†РёРєР»С‹ Р»РѕРІРёС‚ checkPendingTypes. РћС‚ С‚РёРїР°
    ** СЃ СЌС‚РёРј СЃРїРѕСЃРѕР±РѕРј РІ final РЅР°СЃР»РµРґРѕРІР°С‚СЊСЃСЏ РЅРµР»СЊР·СЏ. РЈС‚РІРµСЂР¶РґРµРЅРёСЏ Р±Р°Р·РѕРІРѕРіРѕ С‚РёРїР°
    ** РЅР°СЃР»РµРґСѓСЋС‚СЃСЏ Рё РїСЂРё СЂР°СЃС€РёСЂРµРЅРёРё, Рё РїСЂРё РѕРіСЂР°РЅРёС‡РµРЅРёРё.
    **
    ** The derivation method is remembered in the type, the base type is already set
    ** (readDerivationBase, finishComplexType) вЂ” xsi:type is checked by them.
    ** By this moment the base is complete: cycles are caught by checkPendingTypes.
    ** Deriving from a type with this method in final is not allowed. The assertions
    ** of the base type are inherited both on extension and on restriction.
    */
    const ComplexType* SchemaBuilder::completeBase(const Element& derivation, bool extension, ComplexType& type)
    {
        const ComplexType* base = type.baseType.complex;

        type.derivationMethod = DERIVATION_RESTRICTION;
        if (extension)
            type.derivationMethod = DERIVATION_EXTENSION;
        if (base != nullptr)
            checkFinal(derivation, base->finalSet, base->name, type.derivationMethod);
        if (base != nullptr)
            type.assertions = base->assertions;
        return base;
    }

    // abstract Рё Р·Р°РїСЂРµС‚С‹ block / final СЃРѕСЃС‚Р°РІРЅРѕРіРѕ С‚РёРїР° (РїРѕ СѓРјРѕР»С‡Р°РЅРёСЋ вЂ” РёР· xs:schema)
    // abstract and the block / final prohibitions of a complex type (by default вЂ” from xs:schema)
    void SchemaBuilder::readTypeFlags(const Element& definition, const SchemaSource& source,
        ComplexType& type) const
    {
        type.isAbstract = booleanAttribute(definition, "abstract", false);
        type.blockSet = derivationAttribute(definition, "block", source.blockDefault, COMPLEX_DERIVATIONS_ALL);
        type.finalSet = derivationAttribute(definition, "final", source.finalDefault, COMPLEX_DERIVATIONS_ALL);
    }
}
