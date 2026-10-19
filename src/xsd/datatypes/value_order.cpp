#include "xsd/internal/datatype_checks.hpp"
#include "utils/decimal.hpp"

#include <cmath>

namespace beresta::xsd::detail
{
    // Беззнаковое число с необязательной точкой (знаки и прочее пропускаются)
    // An unsigned number with an optional point (signs and the rest are skipped)
    static long double parseUnsigned(std::string_view text) noexcept
    {
        long double result = 0.0L;
        long double scale = 1.0L;
        bool fraction = false;

        for (char symbol : text)
        {
            if (symbol == '.')
                fraction = true;
            if (symbol < '0' || symbol > '9')
                continue;
            result = result * 10.0L + (symbol - '0');
            if (fraction)
                scale /= 10.0L;
        }
        return result * scale;
    }

    // float / double в long double: INF, -INF, NaN и мантисса с экспонентой
    // float / double to long double: INF, -INF, NaN and a mantissa with an exponent
    long double parseFloatValue(std::string_view value) noexcept
    {
        if (value == "INF")
            return HUGE_VALL;
        if (value == "-INF")
            return -HUGE_VALL;
        if (value == "NaN")
            return std::nan("");
        std::size_t exponentAt = value.find_first_of("eE");
        std::string_view mantissa = value.substr(0, exponentAt);
        long double result = parseUnsigned(mantissa);
        if (exponentAt != std::string_view::npos)
        {
            std::string_view exponent = value.substr(exponentAt + 1);
            long double power = parseUnsigned(exponent);
            if (!exponent.empty() && exponent[0] == '-')
                power = -power;
            result *= std::pow(10.0L, power);
        }
        if (!mantissa.empty() && mantissa[0] == '-')
            return -result;
        return result;
    }

    /*
    ** Дата/время -> одно число секунд в смешанной системе счисления
    ** (месяц по основанию 13, день — 32 и т.д.). Отображение монотонно,
    ** поэтому порядок чисел совпадает с порядком моментов времени,
    ** в том числе после сдвига на часовой пояс. Время без пояса
    ** сравнивается как UTC (упрощение частичного порядка XSD).
    **
    ** Date/time -> one number of seconds in a mixed radix system
    ** (the month in base 13, the day in base 32 and so on). The mapping is monotonic,
    ** so the order of the numbers matches the order of the moments in time,
    ** including after the shift by the time zone. A time without a zone
    ** is compared as UTC (a simplification of the XSD partial order).
    */
    // Ключ порядка: дата и время в секундах, приведённые к UTC
    // The order key: the date and time in seconds, converted to UTC
    long double dateTimeKey(const DateTimeValue& value) noexcept
    {
        long double days = (static_cast<long double>(value.year) * 13 + value.month) * 32 + value.day;
        long double minutes = (days * 24 + value.hour) * 60 + value.minute - value.timezoneMinutes;

        return minutes * 60 + value.second;
    }

    // Сравнение двух длинных чисел с учётом NaN (2 — несравнимы)
    // Compares two long numbers taking NaN into account (2 — incomparable)
    static int compareLongDoubles(long double left, long double right) noexcept
    {
        if (std::isnan(left) || std::isnan(right))
            return 2;
        if (left < right)
            return -1;
        if (left > right)
            return 1;
        return 0;
    }

    // Упорядоченное сравнение значений одного примитивного типа
    // An ordered comparison of values of one primitive type
    int compareOrdered(Primitive primitive, std::string_view left, std::string_view right)
    {
        if (primitive == Primitive::DECIMAL)
            return utils::compareDecimalText(left, right);
        if (primitive == Primitive::FLOAT || primitive == Primitive::DOUBLE)
            return compareLongDoubles(parseFloatValue(left), parseFloatValue(right));
        DateTimeValue a;
        DateTimeValue b;
        if (!parseDateTimeValue(primitive, left, a) || !parseDateTimeValue(primitive, right, b))
            return 2;
        return compareLongDoubles(dateTimeKey(a), dateTimeKey(b));
    }
}
