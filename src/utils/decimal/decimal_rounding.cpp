#include "utils/internal/decimal_digits.hpp"

namespace beresta::utils
{
    // Отбрасывание дробной части (к нулю)
    // Dropping the fraction (towards zero)
    Decimal truncateDecimal(const Decimal& value)
    {
        std::string integer;

        if (value.digits.size() > value.scale)
            integer = value.digits.substr(0, value.digits.size() - value.scale);
        return detail::makeDecimal(value.negative, integer, 0);
    }

    // Округление вниз: у отрицательного числа с дробью — на единицу меньше усечённого
    // Rounding down: for a negative number with a fraction — one less than the truncated one
    Decimal floorDecimal(const Decimal& value)
    {
        Decimal truncated = truncateDecimal(value);

        if (value.negative && value.scale > 0)
            return subtractDecimals(truncated, decimalFromInteger(1));
        return truncated;
    }

    // Округление вверх: у положительного числа с дробью — на единицу больше усечённого
    // Rounding up: for a positive number with a fraction — one more than the truncated one
    Decimal ceilingDecimal(const Decimal& value)
    {
        Decimal truncated = truncateDecimal(value);

        if (!value.negative && value.scale > 0)
            return addDecimals(truncated, decimalFromInteger(1));
        return truncated;
    }

    // round(x) = floor(x + 0.5): половина округляется вверх, как в XPath
    // round(x) = floor(x + 0.5): a half is rounded up, as in XPath
    Decimal roundDecimal(const Decimal& value)
    {
        Decimal half;

        half.digits = "5";
        half.scale = 1;
        return floorDecimal(addDecimals(value, half));
    }
}
