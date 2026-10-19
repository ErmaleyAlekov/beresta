#include "utils/internal/decimal_digits.hpp"

namespace beresta::utils
{
    /*
    ** Сложение со знаками при общем масштабе: одинаковые знаки — модули
    ** складываются, разные — из большего модуля вычитается меньший, и
    ** результат получает знак большего.
    **
    ** Addition with signs at a common scale: equal signs — the absolute values
    ** are added, different ones — the smaller absolute value is subtracted from the larger one,
    ** and the result gets the sign of the larger one.
    */
    Decimal addDecimals(const Decimal& left, const Decimal& right)
    {
        std::size_t scale = left.scale;

        if (right.scale > scale)
            scale = right.scale;
        std::string a = detail::scaledDigits(left, scale);
        std::string b = detail::scaledDigits(right, scale);
        if (left.negative == right.negative)
            return detail::makeDecimal(left.negative, detail::addDigits(a, b), scale);
        if (detail::compareDigits(a, b) >= 0)
            return detail::makeDecimal(left.negative, detail::subtractDigits(a, b), scale);
        return detail::makeDecimal(right.negative, detail::subtractDigits(b, a), scale);
    }

    // Вычитание — сложение с противоположным числом
    // Subtraction is addition of the opposite number
    Decimal subtractDecimals(const Decimal& left, const Decimal& right)
    {
        return addDecimals(left, negateDecimal(right));
    }

    // Произведение: модули перемножаются, масштабы складываются, знак — «минус на плюс»
    // The product: the absolute values are multiplied, the scales are added, the sign — "minus times plus"
    Decimal multiplyDecimals(const Decimal& left, const Decimal& right)
    {
        return detail::makeDecimal(left.negative != right.negative,
            detail::multiplyDigits(left.digits, right.digits), left.scale + right.scale);
    }
}
