#include "utils/internal/decimal_digits.hpp"

#include <utility>

namespace beresta::utils::detail
{
    /*
    ** Нормализация: хвостовые нули дробной части уходят вместе с уменьшением
    ** масштаба, ведущие нули — просто отбрасываются; у нуля нет знака и
    ** масштаба. После этого равные значения имеют одинаковые поля.
    **
    ** Normalization: trailing zeros of the fraction go away together with reducing
    ** the scale, leading zeros are simply dropped; zero has no sign and
    ** no scale. After this equal values have the same fields.
    */
    Decimal makeDecimal(bool negative, std::string digits, std::size_t scale)
    {
        Decimal result;

        while (scale > 0 && !digits.empty() && digits.back() == '0')
        {
            digits.pop_back();
            --scale;
        }
        result.digits = std::string(stripLeadingZeros(digits));
        result.scale = scale;
        result.negative = negative;
        if (result.digits.empty())
        {
            result.scale = 0;
            result.negative = false;
        }
        return result;
    }

    // Цифры с дописанными справа нулями: то же значение при масштабе scale (scale >= value.scale)
    // The digits with zeros appended on the right: the same value at the scale scale (scale >= value.scale)
    std::string scaledDigits(const Decimal& value, std::size_t scale)
    {
        std::string digits = value.digits;

        if (!digits.empty())
            digits.append(scale - value.scale, '0');
        return digits;
    }
}

namespace beresta::utils
{
    /*
    ** Сравнение: сначала знак, затем модули при общем масштабе (у
    ** отрицательных чисел порядок модулей обратный).
    **
    ** Comparison: first the sign, then the absolute values at a common scale (for
    ** negative numbers the order of the absolute values is reversed).
    */
    int compareDecimals(const Decimal& left, const Decimal& right)
    {
        std::size_t scale = left.scale;

        if (left.negative != right.negative)
        {
            if (left.negative)
                return -1;
            return 1;
        }
        if (right.scale > scale)
            scale = right.scale;
        int order = detail::compareDigits(detail::scaledDigits(left, scale), detail::scaledDigits(right, scale));
        if (left.negative)
            return -order;
        return order;
    }

    // Смена знака (у нуля знака нет)
    // Changing the sign (zero has no sign)
    Decimal negateDecimal(const Decimal& value)
    {
        Decimal result = value;

        result.negative = !value.negative && !value.digits.empty();
        return result;
    }
}
