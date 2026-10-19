#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** xs:complexType. Объект регистрируется до построения содержимого
    ** (рекурсивные модели), но помечается незавершённым: наследоваться
    ** от незавершённого типа нельзя — это циклическое наследование.
    ** publish = false — прежнее определение, заменённое xs:redefine: оно
    ** сохраняет имя, но под ним не регистрируется (имя занято новым).
    **
    ** xs:complexType. The object is registered before its content is built
    ** (recursive models), but marked incomplete: deriving from an
    ** incomplete type is not allowed — it is circular derivation.
    ** publish = false — the former definition replaced by xs:redefine: it keeps
    ** its name but is not registered under it (the name belongs to the new one).
    */
    const ComplexType* SchemaBuilder::buildComplexType(const Element& definition,
        const SchemaSource& source, const QualifiedKey& name, bool publish)
    {
        _schema._complexTypes.emplace_back();
        ComplexType& type = _schema._complexTypes.back();
        const Element* simpleContent = firstSchemaChild(definition, "simpleContent");
        const Element* complexContent = firstSchemaChild(definition, "complexContent");
        bool mixed = booleanAttribute(definition, "mixed", false);

        type.name = name.second;
        type.namespaceUri = name.first;
        readTypeFlags(definition, source, type);
        if (publish && !name.second.empty())
        {
            _builtTypes[name] = TypeRef{nullptr, &type};
            _schema._globalTypes[name] = TypeRef{nullptr, &type};
        }
        _incomplete.insert(&type);
        if (simpleContent != nullptr)
            buildSimpleContent(type, *simpleContent, source);
        else if (complexContent != nullptr)
            buildComplexContent(type, *complexContent, source, mixed);
        else
            buildTypeBody(type, definition, source, mixed);
        _incomplete.erase(&type);
        return &type;
    }

    /*
    ** Тело типа: необязательная модель (sequence / choice / all / group),
    ** атрибуты и утверждения xs:assert. Модель заменяет унаследованную только
    ** если она есть; склейку при расширении выполняет buildComplexContent.
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
            type.hasParticle = true;
            break;
        }
        if (type.hasParticle || mixed)
            type.content = ContentType::ELEMENT_ONLY;
        if (mixed)
            type.content = ContentType::MIXED_CONTENT;
        buildAttributes(body, source, type, 0);
        readAssertions(body, source, type);
    }

    // Наследование задаётся единственным xs:extension или xs:restriction
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
    ** Базовый тип (по умолчанию xs:anyType) и способ наследования
    ** запоминаются в типе — по ним проверяется xsi:type. От незавершённого
    ** типа наследоваться нельзя (цикл), от типа с этим способом в final —
    ** тоже. Ссылка на собственное имя в xs:redefine ведёт к прежнему типу.
    ** Утверждения базового типа наследуются и при расширении, и при ограничении.
    **
    ** The base type (xs:anyType by default) and the derivation method
    ** are remembered in the type — xsi:type is checked by them. Deriving from an
    ** incomplete type is not allowed (a cycle), nor from a type with this method in
    ** final. A reference to its own name in xs:redefine leads to the former type.
    ** The assertions of the base type are inherited both on extension and on restriction.
    */
    const ComplexType* SchemaBuilder::completeBase(const Element& derivation, bool extension, ComplexType& type)
    {
        std::optional<std::string> name = derivation.getAttribute("base");

        type.baseType = TypeRef();
        type.derivationMethod = DERIVATION_RESTRICTION;
        if (extension)
            type.derivationMethod = DERIVATION_EXTENSION;
        if (name.has_value())
            type.baseType = referencedType(derivation, name.value());
        const ComplexType* base = type.baseType.complex;
        if (base != nullptr && _incomplete.count(base) != 0)
            fail(derivation, i18n::format(i18n::Message::SCHEMA_CYCLIC_DERIVATION));
        if (base != nullptr)
            checkFinal(derivation, base->finalSet, base->name, type.derivationMethod);
        if (base != nullptr)
            type.assertions = base->assertions;
        return base;
    }

    // abstract и запреты block / final составного типа (по умолчанию — из xs:schema)
    // abstract and the block / final prohibitions of a complex type (by default — from xs:schema)
    void SchemaBuilder::readTypeFlags(const Element& definition, const SchemaSource& source,
        ComplexType& type) const
    {
        type.isAbstract = booleanAttribute(definition, "abstract", false);
        type.blockSet = derivationAttribute(definition, "block", source.blockDefault, COMPLEX_DERIVATIONS_ALL);
        type.finalSet = derivationAttribute(definition, "final", source.finalDefault, COMPLEX_DERIVATIONS_ALL);
    }
}
