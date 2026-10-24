#include "xsd/internal/facet_rules.hpp"

namespace beresta::xsd::detail
{
    // min/max Inclusive/Exclusive
    bool isBoundFacet(std::string_view facet) noexcept
    {
        return facet == "minInclusive" || facet == "maxInclusive" || facet == "minExclusive"
            || facet == "maxExclusive";
    }

    // length, minLength, maxLength
    bool isLengthFacet(std::string_view facet) noexcept
    {
        return facet == "length" || facet == "minLength" || facet == "maxLength";
    }

    // Примитивы, значения которых имеют длину: строки, URI, QName, NOTATION и двоичные данные
    // The primitives whose values have a length: strings, URIs, QName, NOTATION and binary data
    static bool hasLength(Primitive primitive) noexcept
    {
        return primitive == Primitive::STRING || primitive == Primitive::ANY_URI || primitive == Primitive::QNAME
            || primitive == Primitive::NOTATION_TYPE || primitive == Primitive::HEX_BINARY
            || primitive == Primitive::BASE64_BINARY;
    }

    /*
    ** Применимые фасеты (часть 2, §4.1.5 и описания примитивов):
    **  - pattern — у всех; у union, кроме него, только enumeration;
    **  - у list — длины, enumeration и whiteSpace;
    **  - у boolean — только pattern и whiteSpace;
    **  - у строковых и двоичных — длины, enumeration, whiteSpace;
    **  - у decimal — цифры и границы; у float, double, duration и дат — границы.
    **
    ** The applicable facets (part 2, §4.1.5 and the descriptions of the primitives):
    **  - pattern — for all; for union, besides it, only enumeration;
    **  - for list — the lengths, enumeration and whiteSpace;
    **  - for boolean — only pattern and whiteSpace;
    **  - for the string and binary ones — the lengths, enumeration, whiteSpace;
    **  - for decimal — the digits and the bounds; for float, double, duration and dates — the bounds.
    */
    bool facetApplies(const SimpleType& base, std::string_view facet)
    {
        bool digits = facet == "totalDigits" || facet == "fractionDigits";

        if (facet == "pattern")
            return true;
        if (base.variety == Variety::UNION)
            return facet == "enumeration";
        if (facet == "whiteSpace")
            return true;
        if (facet == "enumeration")
            return base.variety == Variety::LIST || base.primitive != Primitive::BOOLEAN;
        if (base.variety == Variety::LIST || hasLength(base.primitive))
            return isLengthFacet(facet);
        if (base.primitive == Primitive::BOOLEAN)
            return false;
        if (base.primitive == Primitive::DECIMAL && digits)
            return true;
        return isBoundFacet(facet);
    }
}
