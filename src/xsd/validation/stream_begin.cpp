#include "xsd/internal/stream_validator.hpp"
#include "i18n/i18n.hpp"
#include "utils/qualified_name.hpp"
#include "xsd/internal/type_derivation.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Проверка элемента с объявлением — в том же порядке, что у обхода
    ** дерева: область ключей, xsi:type (ошибка — дальше не проверяется),
    ** abstract, xsi:nil (принятый — содержимое не проверяется), затем
    ** простой или составной тип. xs:anyType допускает что угодно. Типы
    ** атрибутов запоминаются для ключей и утверждений (аналог PSVI).
    **
    ** Checking an element with a declaration — in the same order as the tree
    ** traversal: the key scope, xsi:type (an error — no further checks),
    ** abstract, xsi:nil (an accepted one — the content is not checked), then
    ** a simple or a complex type. xs:anyType allows anything. The attribute
    ** types are remembered for keys and assertions (an analog of the PSVI).
    */
    void StreamValidator::beginValidation(const SaxElement& element, const ElementDecl& declaration)
    {
        ElementFrame& frame = _frames[_depth - 1];
        TypeRef type = declaration.type;
        XsiValues xsi = findXsi(element);

        frame.declaration = &declaration;
        frame.mode = FrameMode::OPAQUE;
        if (!declaration.identityConstraints.empty())
            openScope(declaration);
        if (!applyXsiType(xsi, declaration, type))
            return;
        bool asserted = type.complex != nullptr && !type.complex->assertions.empty();
        if (_identity != nullptr || recordingAssertions() || asserted)
            frame.attributeTypes.assign(element.attributes.size(), nullptr);
        if (declaration.isAbstract)
            report(_depth - 1, CheckPhase::ABSTRACT, i18n::format(i18n::Message::XSD_ABSTRACT_ELEMENT));
        if (isNil(xsi, declaration))
            beginNil(element, type);
        else if (type.simple != nullptr)
            beginSimple(element, *type.simple);
        else if (type.complex != nullptr)
            beginComplex(element, *type.complex);
    }

    /*
    ** Действующий тип элемента. xsi:type="p:Тип": префикс ищется среди
    ** объявлений открытых элементов, включая этот; тип обязан быть выведен
    ** из объявленного способами, не запрещёнными block элемента и
    ** объявленного типа (cvc-elt.4.3). Абстрактный составной тип для
    ** элемента документа недопустим (cvc-type.2). Ошибка — содержимое
    ** дальше не проверяется. Без xsi:type проверка стоит одно сравнение.
    **
    ** The effective type of the element. xsi:type="p:Type": the prefix is looked up
    ** among the declarations of the open elements, including this one; the type must be
    ** derived from the declared one by methods not prohibited by the block of the element
    ** and of the declared type (cvc-elt.4.3). An abstract complex type for a
    ** document element is not allowed (cvc-type.2). An error — the content
    ** is not checked further. Without xsi:type the check costs one comparison.
    */
    bool StreamValidator::applyXsiType(const XsiValues& xsi, const ElementDecl& declaration, TypeRef& type)
    {
        if (xsi.hasType)
        {
            utils::QualifiedNameParts parts = utils::splitQualifiedName(xsi.type);
            std::string_view namespaceUri = _prefixes.lookup(parts.prefix).value_or(std::string_view());
            bool found = false;
            TypeRef replacement = _schema.findType(namespaceUri, parts.localName, found);
            DerivationSet blocked = declaration.blockSet;
            if (type.complex != nullptr)
                blocked |= type.complex->blockSet;
            bool accepted = found && isDerivedFrom(replacement, type, blocked);
            i18n::Message problem = i18n::Message::XSD_XSI_TYPE_NOT_FOUND;
            if (found)
                problem = i18n::Message::XSD_XSI_TYPE_NOT_DERIVED;
            if (!accepted)
            {
                report(_depth - 1, CheckPhase::XSI_TYPE, i18n::format(problem, {xsi.type}));
                return false;
            }
            type = replacement;
        }
        if (type.complex == nullptr || !type.complex->isAbstract)
            return true;
        report(_depth - 1, CheckPhase::XSI_TYPE, i18n::format(i18n::Message::XSD_ABSTRACT_TYPE, {type.complex->name}));
        return false;
    }

    // xsi:nil="true" допустим только у nillable-элементов; пустоту содержимого проверит закрытие
    // xsi:nil="true" is allowed only for nillable elements; the emptiness of the content is checked on closing
    bool StreamValidator::isNil(const XsiValues& xsi, const ElementDecl& declaration)
    {
        if (!xsi.hasNil || (xsi.nil != "true" && xsi.nil != "1"))
            return false;
        if (!declaration.nillable)
        {
            report(_depth - 1, CheckPhase::NIL, i18n::format(i18n::Message::XSD_NOT_NILLABLE));
            return false;
        }
        return true;
    }

    /*
    ** Принятый xsi:nil="true": содержимое обязано быть пустым, но атрибуты
    ** проверяются как обычно (cvc-elt.3 не отменяет cvc-type): у
    ** составного типа — сразу, у простого — откладываются до закрытия,
    ** где о каждом не-xsi атрибуте сообщит closeNil.
    **
    ** An accepted xsi:nil="true": the content must be empty, but the attributes
    ** are validated as usual (cvc-elt.3 does not cancel cvc-type): for a
    ** complex type — at once, for a simple one — they are deferred until closing,
    ** where closeNil reports every non-xsi attribute.
    */
    void StreamValidator::beginNil(const SaxElement& element, TypeRef type)
    {
        ElementFrame& frame = _frames[_depth - 1];

        if (type.simple != nullptr)
            beginSimple(element, *type.simple);
        else if (type.complex != nullptr)
            validateAttributes(element, *type.complex);
        frame.mode = FrameMode::NIL;
        frame.collectText = false;
    }

    // Составной тип: атрибуты сразу, затем — простое содержимое (текст копится) или курсор модели
    // A complex type: the attributes at once, then — simple content (the text is accumulated) or the model cursor
    void StreamValidator::beginComplex(const SaxElement& element, const ComplexType& type)
    {
        ElementFrame& frame = _frames[_depth - 1];

        frame.mode = FrameMode::COMPLEX;
        frame.complex = &type;
        validateAttributes(element, type);
        if (type.content == ContentType::SIMPLE_CONTENT)
        {
            frame.valueType = type.simpleContent;
            frame.collectText = true;
            return;
        }
        if (!type.hasParticle)
            return;
        frame.plan = planFor(type.particle);
        frame.cursor.reset(type.particle, frame.plan);
    }
}
