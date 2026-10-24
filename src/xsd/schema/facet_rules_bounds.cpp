#include "xsd/internal/facet_rules.hpp"
#include "xsd/internal/datatype_checks.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Правило пары границ: при каком порядке first относительно second
    ** (меньше, равно, больше) пара недопустима. Несравнимые значения
    ** (частичный порядок duration и дат с часовым поясом и без) правило не
    ** нарушают: спецификация запрещает только определённо больший или меньший.
    ** Таблица вместо цепочки if (паттерн «таблица правил»): каждое правило
    ** части 2, §4.3.7–4.3.10 — одна строка.
    **
    ** The rule of a pair of bounds: at which order of first relative to second
    ** (less, equal, greater) the pair is not allowed. Incomparable values
    ** (the partial order of duration and of dates with and without a time zone) do not
    ** break a rule: the specification prohibits only a definitely greater or smaller one.
    ** A table instead of an if chain (the "rule table" pattern): every rule
    ** of part 2, §4.3.7–4.3.10 is one row.
    */
    struct BoundRule
    {
        const char* first;
        const char* second;
        bool below;
        bool equal;
        bool above;
    };

    // Граница шага (first) против границы базы (second) — «действительное ограничение»
    // The step's bound (first) against the base's bound (second) — "valid restriction"
    static const BoundRule RESTRICTION_RULES[] = {
        {"maxInclusive", "maxInclusive", false, false, true}, {"maxInclusive", "maxExclusive", false, true, true},
        {"maxInclusive", "minInclusive", true, false, false}, {"maxInclusive", "minExclusive", true, true, false},
        {"maxExclusive", "maxExclusive", false, false, true}, {"maxExclusive", "maxInclusive", false, false, true},
        {"maxExclusive", "minInclusive", true, true, false}, {"maxExclusive", "minExclusive", true, true, false},
        {"minExclusive", "minExclusive", true, false, false}, {"minExclusive", "maxInclusive", false, false, true},
        {"minExclusive", "minInclusive", true, false, false}, {"minExclusive", "maxExclusive", false, true, true},
        {"minInclusive", "minInclusive", true, false, false}, {"minInclusive", "maxInclusive", false, false, true},
        {"minInclusive", "minExclusive", true, true, false}, {"minInclusive", "maxExclusive", false, true, true}
    };

    // Нижняя граница (first) против верхней (second) в фасетах после шага
    // A lower bound (first) against an upper one (second) in the facets after the step
    static const BoundRule ORDER_RULES[] = {
        {"minInclusive", "maxInclusive", false, false, true}, {"minInclusive", "maxExclusive", false, true, true},
        {"minExclusive", "maxExclusive", false, false, true}, {"minExclusive", "maxInclusive", false, true, true}
    };

    // Значение граничного фасета по имени
    // The value of a bound facet by name
    static const std::optional<std::string>& boundOf(const Facets& facets, std::string_view kind) noexcept
    {
        if (kind == "minInclusive")
            return facets.minInclusive;
        if (kind == "maxInclusive")
            return facets.maxInclusive;
        if (kind == "minExclusive")
            return facets.minExclusive;
        return facets.maxExclusive;
    }

    /*
    ** Нарушает ли пара значений правило. Сообщение — «ослабляет», если обе
    ** границы с одной стороны (обе min* или обе max*), иначе «несовместимы».
    **
    ** Whether a pair of values breaks the rule. The message is "weakens" if both
    ** bounds are on the same side (both min* or both max*), otherwise "incompatible".
    */
    static std::optional<std::string> breakRule(const BoundRule& rule, const std::string& first,
        const std::string& second, Primitive primitive)
    {
        int order = compareOrdered(primitive, first, second);
        bool broken = (order < 0 && rule.below) || (order == 0 && rule.equal) || (order == 1 && rule.above);
        i18n::Message message = i18n::Message::SCHEMA_FACET_ORDER;

        if (!broken)
            return std::nullopt;
        if (std::string_view(rule.first).substr(0, 3) == std::string_view(rule.second).substr(0, 3))
            message = i18n::Message::SCHEMA_FACET_WIDENS;
        return i18n::format(message, {rule.first, first, rule.second, second});
    }

    /*
    ** Значения границ шага: две нижние или две верхние в одном шаге нельзя
    ** (§4.3.7.4, §4.3.9.4), каждое значение — значение базового типа. Границы
    ** базы при этой проверке сняты: их сравнивает checkBoundRestriction
    ** (так maxExclusive="10" допустим под maxExclusive="10" базы).
    **
    ** The values of the step's bounds: two lower or two upper ones in one step are not allowed
    ** (§4.3.7.4, §4.3.9.4), every value is a value of the base type. The base's bounds
    ** are removed for this check: checkBoundRestriction compares them
    ** (so maxExclusive="10" is allowed under the base's maxExclusive="10").
    */
    std::optional<std::string> checkBoundValues(const FacetStep& step)
    {
        SimpleType probe = *step.base;

        if (step.named.count("minInclusive") != 0 && step.named.count("minExclusive") != 0)
            return i18n::format(i18n::Message::SCHEMA_FACET_CONFLICT, {"minInclusive", "minExclusive"});
        if (step.named.count("maxInclusive") != 0 && step.named.count("maxExclusive") != 0)
            return i18n::format(i18n::Message::SCHEMA_FACET_CONFLICT, {"maxInclusive", "maxExclusive"});
        probe.facets.minInclusive.reset();
        probe.facets.maxInclusive.reset();
        probe.facets.minExclusive.reset();
        probe.facets.maxExclusive.reset();
        for (const auto& entry : step.named)
        {
            std::optional<std::string> error;
            if (isBoundFacet(entry.first))
                error = validateSimpleValue(probe, entry.second, step.context);
            if (error.has_value())
                return i18n::format(i18n::Message::SCHEMA_FACET_VALUE_INVALID, {entry.first, error.value()});
        }
        return std::nullopt;
    }

    // Границы шага не выходят за границы базы (таблица RESTRICTION_RULES)
    // The step's bounds do not go beyond the base's bounds (the RESTRICTION_RULES table)
    std::optional<std::string> checkBoundRestriction(const FacetStep& step)
    {
        for (const BoundRule& rule : RESTRICTION_RULES)
        {
            auto own = step.named.find(rule.first);
            const std::optional<std::string>& base = boundOf(step.inherited, rule.second);
            std::optional<std::string> error;
            if (own != step.named.end() && base.has_value())
                error = breakRule(rule, own->second, base.value(), step.base->primitive);
            if (error.has_value())
                return error;
        }
        return std::nullopt;
    }

    // Нижние границы после шага не выше верхних (таблица ORDER_RULES)
    // The lower bounds after the step are not above the upper ones (the ORDER_RULES table)
    std::optional<std::string> checkBoundOrder(const Facets& merged, Primitive primitive)
    {
        for (const BoundRule& rule : ORDER_RULES)
        {
            const std::optional<std::string>& lower = boundOf(merged, rule.first);
            const std::optional<std::string>& upper = boundOf(merged, rule.second);
            std::optional<std::string> error;
            if (lower.has_value() && upper.has_value())
                error = breakRule(rule, lower.value(), upper.value(), primitive);
            if (error.has_value())
                return error;
        }
        return std::nullopt;
    }
}
