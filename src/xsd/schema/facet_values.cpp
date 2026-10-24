#include "xsd/internal/facet_rules.hpp"

namespace beresta::xsd::detail
{
    // Неотрицательное целое значение фасета; некорректное — nullopt
    // A non-negative integer facet value; an invalid one is nullopt
    static std::optional<std::size_t> parseSize(const std::string& value)
    {
        std::size_t result = 0;

        if (value.empty() || value.size() > 18)
            return std::nullopt;
        for (char symbol : value)
        {
            if (symbol < '0' || symbol > '9')
                return std::nullopt;
            result = result * 10 + static_cast<std::size_t>(symbol - '0');
        }
        return result;
    }

    // Фасеты-числа: length, minLength, maxLength, totalDigits, fractionDigits
    // Number facets: length, minLength, maxLength, totalDigits, fractionDigits
    static std::optional<std::size_t>* sizeFacet(Facets& facets, std::string_view kind)
    {
        if (kind == "length")
            return &facets.length;
        if (kind == "minLength")
            return &facets.minLength;
        if (kind == "maxLength")
            return &facets.maxLength;
        if (kind == "totalDigits")
            return &facets.totalDigits;
        if (kind == "fractionDigits")
            return &facets.fractionDigits;
        return nullptr;
    }

    // Фасеты-границы: min/max Inclusive/Exclusive
    // Bound facets: min/max Inclusive/Exclusive
    static std::optional<std::string>* boundFacet(Facets& facets, std::string_view kind)
    {
        if (kind == "minInclusive")
            return &facets.minInclusive;
        if (kind == "maxInclusive")
            return &facets.maxInclusive;
        if (kind == "minExclusive")
            return &facets.minExclusive;
        if (kind == "maxExclusive")
            return &facets.maxExclusive;
        return nullptr;
    }

    /*
    ** Числовые, граничные фасеты и whiteSpace; false — фасет неизвестен или
    ** значение некорректно. totalDigits — positiveInteger: ноль недопустим,
    ** остальные числовые фасеты — nonNegativeInteger.
    **
    ** Number and bound facets and whiteSpace; false — the facet is unknown or
    ** the value is invalid. totalDigits is a positiveInteger: zero is not allowed,
    ** the other number facets are nonNegativeInteger.
    */
    bool applySimpleFacet(Facets& facets, std::string_view kind, const std::string& value)
    {
        std::optional<std::size_t>* size = sizeFacet(facets, kind);
        std::optional<std::string>* bound = boundFacet(facets, kind);

        if (size != nullptr)
        {
            *size = parseSize(value);
            return size->has_value() && (kind != "totalDigits" || size->value() > 0);
        }
        if (bound != nullptr)
        {
            *bound = value;
            return true;
        }
        if (kind != "whiteSpace")
            return false;
        if (value == "preserve")
            facets.whiteSpace = WhiteSpace::PRESERVE;
        else if (value == "replace")
            facets.whiteSpace = WhiteSpace::REPLACE;
        else if (value == "collapse")
            facets.whiteSpace = WhiteSpace::COLLAPSE;
        return value == "preserve" || value == "replace" || value == "collapse";
    }
}
