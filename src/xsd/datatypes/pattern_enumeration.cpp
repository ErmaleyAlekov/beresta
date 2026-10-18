#include "xsd/internal/datatype_checks.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Шаблоны: значение обязано пройти каждую группу (шаг наследования),
    ** а внутри группы достаточно совпасть с любым шаблоном.
    ** Перечисление: значение равно одному из допустимых (по значению).
    **
    ** Patterns: the value must pass each group (a derivation step),
    ** and inside a group matching any pattern is enough.
    ** Enumeration: the value equals one of the allowed ones (by value).
    */
    std::optional<std::string> checkPatternsAndEnumeration(const SimpleType& type,
        std::string_view value)
    {
        for (const std::vector<std::shared_ptr<const Regex>>& group : type.facets.patterns)
        {
            bool matched = false;
            for (const std::shared_ptr<const Regex>& pattern : group)
                matched = matched || pattern->matches(value);
            if (!matched)
                return i18n::format(i18n::Message::PATTERN_MISMATCH, {value, group.front()->getPattern()});
        }
        if (type.facets.enumeration.empty())
            return std::nullopt;
        for (const std::string& allowed : type.facets.enumeration)
        {
            if (valuesEqual(type, value, allowed))
                return std::nullopt;
        }
        return i18n::format(i18n::Message::NOT_IN_ENUMERATION, {value, type.name});
    }

    // Порядок 2 (несравнимы) границу не нарушает
    // Order 2 (incomparable) does not violate a bound
    bool breaksBound(int order, bool lower, bool inclusive) noexcept
    {
        if (order == 2)
            return false;
        if (lower && inclusive)
            return order < 0;
        if (lower)
            return order <= 0;
        if (inclusive)
            return order > 0;
        return order >= 0;
    }
}
