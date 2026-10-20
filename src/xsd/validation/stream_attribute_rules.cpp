#include "xsd/internal/stream_validator.hpp"
#include "xsd/internal/attribute_wildcards.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Атрибут, не объявленный в типе: его должен допускать xs:anyAttribute
    ** (ограничение пространств имён, cvc-wildcard.2). При skip он не
    ** проверяется; при lax и strict — по глобальному объявлению, а без
    ** объявления strict — ошибка. Непроверенный атрибут остаётся без типа
    ** (для ключей). Результат — 1, если атрибут проверен как значение типа
    ** ID (для cvc-complex-type.5), иначе 0.
    **
    ** An attribute not declared in the type: xs:anyAttribute must admit it
    ** (the namespace constraint, cvc-wildcard.2). With skip it is not
    ** validated; with lax and strict — by the global declaration, and without a
    ** declaration strict is an error. An unvalidated attribute stays without a type
    ** (for keys). The result is 1 if the attribute was validated as a value of the
    ** ID type (for cvc-complex-type.5), otherwise 0.
    */
    std::size_t StreamValidator::checkWildcardAttribute(const ComplexType& type, const SaxAttribute& attribute,
        std::size_t index)
    {
        const AttributeWildcard& wildcard = type.attributeWildcard;

        if (!type.anyAttribute || !wildcardAllows(wildcard, attribute.namespaceUri))
        {
            report(_depth - 1, CheckPhase::ATTRIBUTES, i18n::format(i18n::Message::XSD_ATTRIBUTE_UNDECLARED,
                {attribute.name}));
            return 0;
        }
        const AttributeUse* global = nullptr;
        if (wildcard.process != ProcessContents::SKIP_CHECK)
            global = _schema.findAttribute(attribute.namespaceUri, attribute.localName);
        if (global == nullptr && wildcard.process == ProcessContents::STRICT_CHECK)
            report(_depth - 1, CheckPhase::ATTRIBUTES, i18n::format(
                i18n::Message::XSD_ATTRIBUTE_WILDCARD_UNDECLARED, {attribute.name}));
        if (global == nullptr)
            _frames[_depth - 1].untypedAttributes.push_back(index);
        if (global == nullptr)
            return 0;
        checkAttribute(*global, attribute, index);
        if (global->type->form == LexicalForm::ID)
            return 1;
        return 0;
    }

    /*
    ** cvc-complex-type.5: из шаблона допустим не более чем один атрибут типа
    ** ID, и только если среди объявленных в типе атрибутов нет атрибута типа ID.
    **
    ** cvc-complex-type.5: at most one attribute of the ID type is allowed from the wildcard,
    ** and only if there is no attribute of the ID type among the ones declared in the type.
    */
    void StreamValidator::checkWildcardIds(const ComplexType& type, std::size_t wildIds)
    {
        bool declared = false;

        if (wildIds == 0)
            return;
        for (const AttributeUse& use : type.attributes)
        {
            if (!use.prohibited && use.type->form == LexicalForm::ID)
                declared = true;
        }
        if (wildIds > 1 || declared)
            report(_depth - 1, CheckPhase::ATTRIBUTES, i18n::format(i18n::Message::XSD_WILDCARD_ID_SEVERAL));
    }

    /*
    ** Атрибута нет в экземпляре: обязательный — ошибка; со значением
    ** default / fixed — значение подставляется (cvc-complex-type.2.4 / 3.4.4,
    ** атрибуты по умолчанию в PSVI): его видят поля ключей, и он
    ** учитывается среди ID / IDREF документа.
    **
    ** The attribute is absent in the instance: a required one is an error; with a
    ** default / fixed value — the value is supplied (cvc-complex-type.2.4 / 3.4.4,
    ** default attributes in the PSVI): key fields see it, and it is
    ** counted among the IDs / IDREFs of the document.
    */
    void StreamValidator::absentAttribute(const AttributeUse& use)
    {
        if (use.required)
        {
            report(_depth - 1, CheckPhase::ATTRIBUTES, i18n::format(i18n::Message::ATTRIBUTE_REQUIRED_MISSING,
                {use.name}));
            return;
        }
        _frames[_depth - 1].defaultAttributes.push_back(&use);
        if (use.defaultValue.has_value())
            recordIdentity(*use.type, use.defaultValue.value(), CheckPhase::ATTRIBUTES);
        else if (use.fixedValue.has_value())
            recordIdentity(*use.type, use.fixedValue.value(), CheckPhase::ATTRIBUTES);
    }
}
