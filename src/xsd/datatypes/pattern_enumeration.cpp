#include "xsd/internal/datatype_checks.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Шаблоны: значение обязано пройти каждую группу (шаг наследования),
    ** а внутри группы достаточно совпасть с любым шаблоном.
    ** Перечисление: значение равно одному из допустимых (по значению).
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
                return "значение \"" + std::string(value) + "\" не соответствует шаблону \""
                    + group.front()->getPattern() + "\"";
        }
        if (type.facets.enumeration.empty())
            return std::nullopt;
        for (const std::string& allowed : type.facets.enumeration)
        {
            if (valuesEqual(type, value, allowed))
                return std::nullopt;
        }
        return "значение \"" + std::string(value) + "\" не входит в перечисление типа " + type.name;
    }

    // Порядок 2 (несравнимы) границу не нарушает
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
