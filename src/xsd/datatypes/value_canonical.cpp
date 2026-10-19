#include "xsd/internal/datatype_checks.hpp"
#include "utils/decimal.hpp"
#include "utils/xml_tokens.hpp"

#include <cmath>
#include <cstdio>
#include <string>

namespace beresta::xsd::detail
{
    // Каноническая запись decimal: "-12.5", "0", "3" (без лишних нулей и знака у нуля)
    // The canonical form of decimal: "-12.5", "0", "3" (without extra zeros and a sign for zero)
    static std::string canonicalDecimal(std::string_view value)
    {
        utils::DecimalParts parts = utils::splitDecimal(value);
        std::string result;

        if (parts.negative)
            result += '-';
        if (parts.integer.empty())
            result += '0';
        else
            result += parts.integer;
        if (!parts.fraction.empty())
        {
            result += '.';
            result += parts.fraction;
        }
        return result;
    }

    /*
    ** Каноническая запись float / double. Число сначала приводится к
    ** точности своего типа (float — 32 бита, double — 64), чтобы "0.1" и
    ** "0.10" дали одно значение, затем печатается с полной точностью.
    ** +0 и -0 в XSD 1.0 равны, поэтому оба дают "0".
    **
    ** The canonical form of float / double. The number is first converted to
    ** the precision of its type (float — 32 bits, double — 64) so that "0.1" and
    ** "0.10" give one value, then printed with full precision.
    ** +0 and -0 are equal in XSD 1.0, so both give "0".
    */
    static std::string canonicalFloat(Primitive primitive, std::string_view value)
    {
        long double number = parseFloatValue(value);
        char buffer[64];

        if (std::isnan(number))
            return "NaN";
        if (std::isinf(number) && number > 0)
            return "INF";
        if (std::isinf(number))
            return "-INF";
        if (number == 0)
            return "0";
        if (primitive == Primitive::FLOAT)
            std::snprintf(buffer, sizeof(buffer), "%.9g", static_cast<double>(static_cast<float>(number)));
        else
            std::snprintf(buffer, sizeof(buffer), "%.17g", static_cast<double>(number));
        return buffer;
    }

    /*
    ** Дата и время: момент, приведённый к UTC (целые секунды и дробная
    ** часть отдельно — так точность не зависит от размера long double),
    ** плюс признак часового пояса: по XSD 1.0 значение с поясом и без
    ** пояса считаются разными.
    **
    ** Date and time: the moment converted to UTC (whole seconds and the fraction
    ** separately — so the precision does not depend on the size of long double),
    ** plus a time zone flag: by XSD 1.0 a value with a time zone and one without
    ** are considered different.
    */
    static std::string canonicalDateTime(Primitive primitive, std::string_view value)
    {
        DateTimeValue parsed;

        if (!parseDateTimeValue(primitive, value, parsed))
            return std::string(value);
        long double key = dateTimeKey(parsed);
        long double whole = std::floor(key);
        char fraction[32];
        std::snprintf(fraction, sizeof(fraction), "%.9f", static_cast<double>(key - whole));
        std::string result = std::to_string(static_cast<long long>(whole)) + (fraction + 1);
        if (parsed.hasTimezone)
            return result + "Z";
        return result + "L";
    }

    // Атомарное значение: префикс примитивного типа + каноническая запись
    // An atomic value: the primitive type prefix + the canonical form
    std::string canonicalAtomic(Primitive primitive, const std::string& value)
    {
        std::string prefix = std::to_string(static_cast<int>(primitive)) + "|";

        if (primitive == Primitive::BOOLEAN && (value == "true" || value == "1"))
            return prefix + "true";
        if (primitive == Primitive::BOOLEAN)
            return prefix + "false";
        if (primitive == Primitive::DECIMAL && isDecimalLexical(value, false))
            return prefix + canonicalDecimal(value);
        if (primitive == Primitive::FLOAT || primitive == Primitive::DOUBLE)
            return prefix + canonicalFloat(primitive, value);
        if (primitive >= Primitive::DATE_TIME && primitive <= Primitive::G_MONTH)
            return prefix + canonicalDateTime(primitive, value);
        if (primitive != Primitive::HEX_BINARY)
            return prefix + value;
        std::string upper = value;
        for (char& symbol : upper)
        {
            if (symbol >= 'a' && symbol <= 'f')
                symbol = static_cast<char>(symbol - 'a' + 'A');
        }
        return prefix + upper;
    }
}

namespace beresta::xsd
{
    /*
    ** Каноническое значение с учётом разновидности типа: у объединения —
    ** по первому члену, которому значение подходит (как при проверке),
    ** у списка — канонические элементы через один пробел.
    **
    ** The canonical value taking the type variety into account: for a union —
    ** by the first member the value fits (as in the check),
    ** for a list — the canonical items separated by one space.
    */
    std::string canonicalValue(const SimpleType& type, std::string_view value)
    {
        std::string normalized = normalizeWhiteSpace(value, type.facets.whiteSpace);

        if (type.variety == Variety::UNION)
        {
            for (const SimpleType* member : type.memberTypes)
            {
                if (!validateSimpleValue(*member, normalized, nullptr).has_value())
                    return canonicalValue(*member, normalized);
            }
            return normalized;
        }
        if (type.variety == Variety::LIST && type.itemType != nullptr)
        {
            std::string result;
            for (std::string_view item : utils::splitTokens(normalized))
            {
                if (!result.empty())
                    result += ' ';
                result += canonicalValue(*type.itemType, item);
            }
            return result;
        }
        return detail::canonicalAtomic(type.primitive, normalized);
    }
}
