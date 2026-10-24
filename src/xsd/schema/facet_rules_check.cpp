#include "xsd/internal/facet_rules.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    // Каждый фасет шага (и перечисление) должен быть применим к базовому типу
    // Every facet of the step (and the enumeration) must be applicable to the base type
    static std::optional<std::string> checkApplicability(const FacetStep& step)
    {
        for (const auto& entry : step.named)
        {
            if (!facetApplies(*step.base, entry.first))
                return i18n::format(i18n::Message::SCHEMA_FACET_NOT_APPLICABLE, {entry.first, step.base->name});
        }
        if (!step.enumeration.empty() && !facetApplies(*step.base, "enumeration"))
            return i18n::format(i18n::Message::SCHEMA_FACET_NOT_APPLICABLE, {"enumeration", step.base->name});
        return std::nullopt;
    }

    // Запись правила whiteSpace для сообщения
    // The notation of a whiteSpace rule for a message
    static std::string_view whiteSpaceName(WhiteSpace mode) noexcept
    {
        if (mode == WhiteSpace::PRESERVE)
            return "preserve";
        if (mode == WhiteSpace::REPLACE)
            return "replace";
        return "collapse";
    }

    /*
    ** whiteSpace нельзя ослабить: collapse базы остаётся collapse, replace —
    ** не preserve (§4.3.6.4). У list и у нестроковых примитивов база уже
    ** collapse, поэтому другое значение там тоже ошибка.
    **
    ** whiteSpace cannot be weakened: the base's collapse stays collapse, replace —
    ** not preserve (§4.3.6.4). For list and for non-string primitives the base is already
    ** collapse, so another value there is an error too.
    */
    static std::optional<std::string> checkWhiteSpaceStep(const FacetStep& step, const Facets& merged)
    {
        if (step.named.count("whiteSpace") == 0 || merged.whiteSpace >= step.inherited.whiteSpace)
            return std::nullopt;
        return i18n::format(i18n::Message::SCHEMA_FACET_WIDENS, {"whiteSpace", whiteSpaceName(merged.whiteSpace),
            "whiteSpace", whiteSpaceName(step.inherited.whiteSpace)});
    }

    // Значения перечисления — значения базового типа (со всеми его фасетами)
    // The enumeration values are values of the base type (with all its facets)
    static std::optional<std::string> checkEnumerationValues(const FacetStep& step)
    {
        for (const std::string& value : step.enumeration)
        {
            std::optional<std::string> error = validateSimpleValue(*step.base, value, step.context);
            if (error.has_value())
                return i18n::format(i18n::Message::SCHEMA_FACET_VALUE_INVALID, {"enumeration", error.value()});
        }
        return std::nullopt;
    }

    /*
    ** Правила по порядку: применимость, whiteSpace, длины, цифры, значения
    ** границ, границы против базы, порядок границ, перечисление. У
    ** ограничения anySimpleType фасетов нет и проверять нечего.
    **
    ** The rules in order: applicability, whiteSpace, lengths, digits, the values
    ** of the bounds, the bounds against the base, the order of the bounds, the enumeration. A
    ** restriction of anySimpleType has no facets and there is nothing to check.
    */
    std::optional<std::string> checkFacetStep(const FacetStep& step, const SimpleType& type)
    {
        std::optional<std::string> error;

        if (step.base == nullptr || step.base->baseType == nullptr)
            return std::nullopt;
        error = checkApplicability(step);
        if (!error.has_value())
            error = checkWhiteSpaceStep(step, type.facets);
        if (!error.has_value())
            error = checkLengthFacets(step, type.facets);
        if (!error.has_value())
            error = checkDigitFacets(step, type.facets);
        if (!error.has_value())
            error = checkBoundValues(step);
        if (!error.has_value())
            error = checkBoundRestriction(step);
        if (!error.has_value())
            error = checkBoundOrder(type.facets, step.base->primitive);
        if (!error.has_value())
            error = checkEnumerationValues(step);
        return error;
    }
}
