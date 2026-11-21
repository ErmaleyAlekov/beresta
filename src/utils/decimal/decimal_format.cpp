#include "utils/decimal.hpp"

#include <cmath>

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

    // Умножение на 10^exponent шагами по 10^300 (10^300 представимо даже в double), пока значение конечно и не ноль
    // Multiplication by 10^exponent in steps of 10^300 (10^300 fits even into double) while the value is finite and nonzero
    static long double scaleByPowerOfTen(long double value, long long exponent) noexcept
    {
        while (exponent != 0 && value != 0.0L && std::isfinite(value))
        {
            long long step = exponent;
            if (step > 300)
                step = 300;
            if (step < -300)
                step = -300;
            if (step > 0)
                value *= std::pow(10.0L, static_cast<long double>(step));
            else
                value /= std::pow(10.0L, static_cast<long double>(-step));
            exponent -= step;
        }
        return value;
    }

    /*
    ** Ведущие нули значащими не считаются; цифры после 36-й значащей
    ** отбрасываются (в целой части они только увеличивают порядок).
    **
    ** Leading zeros are not counted as significant; the digits after the 36th significant one
    ** are dropped (in the integer part they only increase the exponent).
    */
    long double digitsToLongDouble(std::string_view digits, long long exponent) noexcept
    {
        long double mantissa = 0.0L;
        std::size_t significant = 0;
        bool fraction = false;

        for (char digit : digits)
        {
            if (digit == '.')
            {
                fraction = true;
                continue;
            }
            if (significant >= 36)
            {
                if (!fraction)
                    ++exponent;
                continue;
            }
            mantissa = mantissa * 10.0L + static_cast<long double>(digit - '0');
            if (mantissa != 0.0L)
                ++significant;
            if (fraction)
                --exponent;
        }
        return scaleByPowerOfTen(mantissa, exponent);
    }

    // Ближайшее double; масштаб больше 10^15 знаков всё равно даёт ноль, поэтому он ограничивается
    // The nearest double; a scale over 10^15 digits gives zero anyway, so it is capped
    double decimalToDouble(const Decimal& value) noexcept
    {
        long long scale = 1000000000000000LL;

        if (value.scale < static_cast<std::size_t>(scale))
            scale = static_cast<long long>(value.scale);
        long double result = digitsToLongDouble(value.digits, -scale);
        if (value.negative)
            return -static_cast<double>(result);
        return static_cast<double>(result);
    }
}
