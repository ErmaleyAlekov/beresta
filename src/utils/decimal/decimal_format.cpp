#include "utils/decimal.hpp"

namespace beresta::utils
{
    /*
    ** Каноническая запись XPath 2.0: знак только у отрицательных, целое —
    ** без точки, дробь — без хвостовых нулей (их нет после нормализации),
    ** у чисел меньше единицы — ведущий "0.".
    **
    ** The canonical XPath 2.0 notation: a sign only for negative numbers, an integer —
    ** without a point, a fraction — without trailing zeros (there are none after normalization),
    ** numbers less than one get a leading "0.".
    */
    std::string formatDecimal(const Decimal& value)
    {
        std::string text;

        if (value.digits.empty())
            return "0";
        if (value.negative)
            text = "-";
        if (value.scale == 0)
            return text + value.digits;
        if (value.digits.size() <= value.scale)
            return text + "0." + std::string(value.scale - value.digits.size(), '0') + value.digits;
        std::size_t integer = value.digits.size() - value.scale;
        return text + value.digits.substr(0, integer) + "." + value.digits.substr(integer);
    }

    /*
    ** Ближайшее double: цифры копятся в long double, затем делятся на
    ** 10^scale. Нужна только для смешивания с xs:double (там точность и
    ** так теряется) — точная арифметика double не использует.
    **
    ** The nearest double: the digits accumulate in a long double, then are divided by
    ** 10^scale. It is needed only for mixing with xs:double (precision is lost
    ** there anyway) — exact arithmetic does not use double.
    */
    double decimalToDouble(const Decimal& value) noexcept
    {
        long double result = 0.0L;
        long double divisor = 1.0L;

        for (char digit : value.digits)
            result = result * 10.0L + static_cast<long double>(digit - '0');
        for (std::size_t index = 0; index < value.scale; ++index)
            divisor *= 10.0L;
        result /= divisor;
        if (value.negative)
            return -static_cast<double>(result);
        return static_cast<double>(result);
    }
}
