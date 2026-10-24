#include "xsd/internal/facet_rules.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    // Сообщение «фасет own ослабляет фасет базы» или «фасеты несовместимы» для числовых фасетов
    // The message "the facet own weakens the base's facet" or "the facets are incompatible" for number facets
    static std::string sizeMessage(i18n::Message message, std::string_view first, std::size_t firstValue,
        std::string_view second, std::size_t secondValue)
    {
        return i18n::format(message, {first, std::to_string(firstValue), second, std::to_string(secondValue)});
    }

    /*
    ** Длины шага против базы: length в одном шаге с minLength / maxLength
    ** нельзя (§4.3.1.4), length базы менять нельзя, minLength — только
    ** увеличить, maxLength — только уменьшить.
    **
    ** The step's lengths against the base: length in one step with minLength / maxLength
    ** is not allowed (§4.3.1.4), the base's length cannot be changed, minLength can only
    ** be increased, maxLength — only decreased.
    */
    static std::optional<std::string> lengthRestriction(const FacetStep& step, const Facets& merged)
    {
        const Facets& base = step.inherited;
        bool ownLength = step.named.count("length") != 0;

        if (ownLength && step.named.count("minLength") != 0)
            return i18n::format(i18n::Message::SCHEMA_FACET_CONFLICT, {"length", "minLength"});
        if (ownLength && step.named.count("maxLength") != 0)
            return i18n::format(i18n::Message::SCHEMA_FACET_CONFLICT, {"length", "maxLength"});
        if (ownLength && base.length.has_value() && *merged.length != *base.length)
            return sizeMessage(i18n::Message::SCHEMA_FACET_WIDENS, "length", *merged.length, "length", *base.length);
        if (step.named.count("minLength") != 0 && base.minLength.has_value() && *merged.minLength < *base.minLength)
            return sizeMessage(i18n::Message::SCHEMA_FACET_WIDENS, "minLength", *merged.minLength,
                "minLength", *base.minLength);
        if (step.named.count("maxLength") != 0 && base.maxLength.has_value() && *merged.maxLength > *base.maxLength)
            return sizeMessage(i18n::Message::SCHEMA_FACET_WIDENS, "maxLength", *merged.maxLength,
                "maxLength", *base.maxLength);
        return std::nullopt;
    }

    /*
    ** Длины после шага согласованы: minLength <= maxLength, а length лежит
    ** между ними (сюда попадают и length шага при minLength базы).
    **
    ** The lengths after the step are consistent: minLength <= maxLength, and length lies
    ** between them (this also covers the step's length with the base's minLength).
    */
    std::optional<std::string> checkLengthFacets(const FacetStep& step, const Facets& merged)
    {
        std::optional<std::string> error = lengthRestriction(step, merged);

        if (error.has_value())
            return error;
        if (merged.minLength.has_value() && merged.maxLength.has_value() && *merged.minLength > *merged.maxLength)
            return sizeMessage(i18n::Message::SCHEMA_FACET_ORDER, "minLength", *merged.minLength,
                "maxLength", *merged.maxLength);
        if (merged.length.has_value() && merged.minLength.has_value() && *merged.length < *merged.minLength)
            return sizeMessage(i18n::Message::SCHEMA_FACET_ORDER, "length", *merged.length,
                "minLength", *merged.minLength);
        if (merged.length.has_value() && merged.maxLength.has_value() && *merged.length > *merged.maxLength)
            return sizeMessage(i18n::Message::SCHEMA_FACET_ORDER, "length", *merged.length,
                "maxLength", *merged.maxLength);
        return std::nullopt;
    }

    /*
    ** Цифры: totalDigits и fractionDigits можно только уменьшить, и
    ** fractionDigits не больше totalDigits. У xs:integer и выведенных из него
    ** fractionDigits="0" зафиксирован (часть 2, §3.3.13); в фасетах встроенного
    ** типа его нет, чтобы не считать цифры каждого целого при проверке.
    **
    ** Digits: totalDigits and fractionDigits can only be decreased, and
    ** fractionDigits is not greater than totalDigits. For xs:integer and the types derived from it
    ** fractionDigits="0" is fixed (part 2, §3.3.13); it is absent from the facets of the built-in
    ** type so as not to count the digits of every integer during validation.
    */
    std::optional<std::string> checkDigitFacets(const FacetStep& step, const Facets& merged)
    {
        Facets base = step.inherited;

        if (step.base->form == LexicalForm::INTEGER)
            base.fractionDigits = 0;

        if (step.named.count("totalDigits") != 0 && base.totalDigits.has_value()
            && *merged.totalDigits > *base.totalDigits)
            return sizeMessage(i18n::Message::SCHEMA_FACET_WIDENS, "totalDigits", *merged.totalDigits,
                "totalDigits", *base.totalDigits);
        if (step.named.count("fractionDigits") != 0 && base.fractionDigits.has_value()
            && *merged.fractionDigits > *base.fractionDigits)
            return sizeMessage(i18n::Message::SCHEMA_FACET_WIDENS, "fractionDigits", *merged.fractionDigits,
                "fractionDigits", *base.fractionDigits);
        if (merged.totalDigits.has_value() && merged.fractionDigits.has_value()
            && *merged.fractionDigits > *merged.totalDigits)
            return sizeMessage(i18n::Message::SCHEMA_FACET_ORDER, "fractionDigits", *merged.fractionDigits,
                "totalDigits", *merged.totalDigits);
        return std::nullopt;
    }
}
