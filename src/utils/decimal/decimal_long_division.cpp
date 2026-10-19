#include "utils/internal/decimal_digits.hpp"

namespace beresta::utils::detail
{
    // Ведущие нули отбрасываются; строка из одних нулей становится пустой (ноль)
    // Leading zeros are dropped; a string of zeros only becomes empty (zero)
    std::string_view stripLeadingZeros(std::string_view digits) noexcept
    {
        std::size_t first = digits.find_first_not_of('0');

        if (first == std::string_view::npos)
            return std::string_view();
        return digits.substr(first);
    }

    /*
    ** Очередная цифра частного: сколько раз делитель помещается в текущий
    ** остаток (не больше девяти, ведь остаток меньше делителя · 10).
    ** Остаток уменьшается на найденное число делителей.
    **
    ** The next digit of the quotient: how many times the divisor fits into the current
    ** remainder (at most nine, because the remainder is less than divisor · 10).
    ** The remainder is reduced by the found number of divisors.
    */
    static char nextQuotientDigit(std::string& remainder, std::string_view divisor)
    {
        char digit = '0';

        while (compareDigits(remainder, divisor) >= 0)
        {
            remainder = subtractDigits(remainder, divisor);
            ++digit;
        }
        return digit;
    }

    /*
    ** Деление столбиком: цифры делимого по одной сносятся в остаток,
    ** каждая даёт одну цифру частного. Делитель обязан быть ненулевым —
    ** это проверяет вызывающий (деление на ноль — ошибка XPath).
    **
    ** Long division: the digits of the dividend are brought down into the remainder
    ** one by one, each gives one digit of the quotient. The divisor must be non-zero —
    ** the caller checks it (division by zero is an XPath error).
    */
    std::string divideDigits(std::string_view dividend, std::string_view divisor, std::string& remainder)
    {
        std::string_view normalizedDivisor = stripLeadingZeros(divisor);
        std::string quotient;

        remainder.clear();
        for (char digit : dividend)
        {
            remainder.push_back(digit);
            remainder = std::string(stripLeadingZeros(remainder));
            quotient.push_back(nextQuotientDigit(remainder, normalizedDivisor));
        }
        return std::string(stripLeadingZeros(quotient));
    }
}
