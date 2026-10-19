#include "utils/internal/decimal_digits.hpp"

namespace beresta::utils
{
    /*
    ** Частное с scale цифрами дробной части. Если left = A·10^(−a) и
    ** right = B·10^(−b), то частное·10^scale = A·10^(b+scale) / (B·10^a) —
    ** одно целочисленное деление столбиком; цифры сверх scale отбрасываются
    ** (усечение к нулю). Точность деления в XPath 2.0 определяется
    ** реализацией; вызывающий выбирает её сам.
    **
    ** The quotient with scale fraction digits. If left = A·10^(−a) and
    ** right = B·10^(−b), then quotient·10^scale = A·10^(b+scale) / (B·10^a) —
    ** one integer long division; the digits beyond scale are dropped
    ** (truncation towards zero). The division precision in XPath 2.0 is
    ** implementation-defined; the caller chooses it.
    */
    bool divideDecimals(const Decimal& left, const Decimal& right, std::size_t scale, Decimal& result)
    {
        std::string remainder;

        if (right.digits.empty())
            return false;
        std::string dividend = left.digits;
        std::string divisor = right.digits;
        dividend.append(right.scale + scale, '0');
        divisor.append(left.scale, '0');
        result = detail::makeDecimal(left.negative != right.negative,
            detail::divideDigits(dividend, divisor, remainder), scale);
        return true;
    }

    /*
    ** Остаток: при общем масштабе оба числа — целые A и B, а остаток от
    ** деления модулей A на B — модуль результата; знак — как у делимого
    ** (так определяют mod и XPath 1.0, и XPath 2.0).
    **
    ** The remainder: at a common scale both numbers are integers A and B, and the remainder
    ** of dividing the absolute values A by B is the absolute value of the result; the sign is
    ** the dividend's (this is how both XPath 1.0 and XPath 2.0 define mod).
    */
    bool moduloDecimals(const Decimal& left, const Decimal& right, Decimal& result)
    {
        std::size_t scale = left.scale;
        std::string remainder;

        if (right.digits.empty())
            return false;
        if (right.scale > scale)
            scale = right.scale;
        detail::divideDigits(detail::scaledDigits(left, scale), detail::scaledDigits(right, scale), remainder);
        result = detail::makeDecimal(left.negative, remainder, scale);
        return true;
    }
}
