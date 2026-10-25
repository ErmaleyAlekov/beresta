#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** xs:complexType. Объект регистрируется до построения содержимого
    ** (рекурсивные модели) и помечается незавершённым. Если базовый тип ещё
    ** строится (его модель ссылается на элемент этого типа — паттерн Composite),
    ** содержимое откладывается до завершения базы (schema_builder_pending.cpp):
    ** это рекурсивная структура данных, а не цикл наследования.
    ** publish = false — прежнее определение, заменённое xs:redefine: оно
    ** сохраняет имя, но под ним не регистрируется (имя занято новым).
    **
    ** xs:complexType. The object is registered before its content is built
    ** (recursive models) and marked incomplete. If the base type is still being
    ** built (its model refers to an element of this type — the Composite pattern),
    ** the content is deferred until the base is complete (schema_builder_pending.cpp):
    ** it is a recursive data structure, not a derivation cycle.
    ** publish = false — the former definition replaced by xs:redefine: it keeps
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
        applyAttributeWildcard(type, buildAttributes(body, source, type, 0));
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
    ** Способ наследования запоминается в типе, базовый тип уже записан
    ** (readDerivationBase, finishComplexType) — по ним проверяется xsi:type.
    ** К этому моменту база завершена: циклы ловит checkPendingTypes. От типа
    ** с этим способом в final наследоваться нельзя. Утверждения базового типа
    ** наследуются и при расширении, и при ограничении.
    **
    ** The derivation method is remembered in the type, the base type is already set
    ** (readDerivationBase, finishComplexType) — xsi:type is checked by them.
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
