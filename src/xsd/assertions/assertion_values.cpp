#include "xsd/internal/assertion_values.hpp"
#include "xsd/internal/datatype_checks.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Член объединения, которому подходит текст (первый по порядку, как
    ** при проверке значения); nullptr — ни один не подошёл.
    **
    ** The union member the text fits (the first in order, as
    ** in value validation); nullptr — none fits.
    */
    static const SimpleType* matchingMember(const SimpleType& type, std::string_view text)
    {
        for (const SimpleType* member : type.memberTypes)
        {
            if (!validateSimpleValue(*member, text, static_cast<const Element*>(nullptr)))
                return member;
        }
        return nullptr;
    }

    // Атомарное значение по примитиву; текст не по лексике — строка (нетипизированное значение)
    // An atomic value by the primitive; a text not by the lexis — a string (an untyped value)
    static xpath::XPathValue atomicValue(Primitive primitive, std::string_view value)
    {
        utils::Decimal exact;
        xpath::XPathValue date = xpath::XPathValue::fromBoolean(false);

        if (primitive == Primitive::DECIMAL && utils::readDecimal(value, exact))
            return xpath::XPathValue::fromDecimal(exact);
        if ((primitive == Primitive::FLOAT || primitive == Primitive::DOUBLE) && isFloatLexical(value))
            return xpath::XPathValue::fromNumber(static_cast<double>(parseFloatValue(value)));
        if (primitive == Primitive::BOOLEAN && isBooleanLexical(value))
            return xpath::XPathValue::fromBoolean(value == "true" || value == "1");
        if (primitive == Primitive::DATE && parseDateAtom(xpath::ValueType::DATE, value, date))
            return date;
        if (primitive == Primitive::DATE_TIME && parseDateAtom(xpath::ValueType::DATE_TIME, value, date))
            return date;
        return xpath::XPathValue::fromString(std::string(value));
    }

    // Объединения раскрываются циклом (объединение объединений — тоже), затем — атом по примитиву
    // Unions are unfolded by a loop (a union of unions too), then — an atom by the primitive
    xpath::XPathValue typedValueOf(const SimpleType& type, std::string_view text)
    {
        const SimpleType* actual = &type;
        std::string buffer;

        while (actual != nullptr && actual->variety == Variety::UNION)
            actual = matchingMember(*actual, text);
        if (actual == nullptr || actual->variety != Variety::ATOMIC)
            return xpath::XPathValue::fromString(std::string(text));
        return atomicValue(actual->primitive, normalizedView(text, actual->facets.whiteSpace, buffer));
    }

    /*
    ** Дата по лексике XSD (те же правила, что у проверки значений) и её
    ** ключ порядка — момент в UTC; время без пояса считается UTC.
    **
    ** A date by the XSD lexis (the same rules as in value validation) and its
    ** order key — the moment in UTC; a time without a zone is considered UTC.
    */
    bool parseDateAtom(xpath::ValueType kind, std::string_view text, xpath::XPathValue& result)
    {
        Primitive primitive = Primitive::DATE_TIME;
        DateTimeValue moment;
        std::string buffer;
        std::string_view value = normalizedView(text, WhiteSpace::COLLAPSE, buffer);

        if (kind == xpath::ValueType::DATE)
            primitive = Primitive::DATE;
        if (!parseDateTimeValue(primitive, value, moment))
            return false;
        result = xpath::XPathValue::fromDate(kind, std::string(value), dateTimeKey(moment));
        return true;
    }
}
