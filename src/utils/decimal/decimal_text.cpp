#include "utils/internal/decimal_digits.hpp"
#include "utils/xml_chars.hpp"

#include <cmath>

namespace beresta::utils
{
    // Пробелы XML по краям (значение xs:decimal сворачивает пробелы)
    // XML spaces around (an xs:decimal value collapses spaces)
    static std::string_view trimSpaces(std::string_view text) noexcept
    {
        while (!text.empty() && isWhitespace(text.front()))
            text.remove_prefix(1);
        while (!text.empty() && isWhitespace(text.back()))
            text.remove_suffix(1);
        return text;
    }

    /*
    ** Лексика xs:decimal: необязательный знак, цифры, необязательная
    ** точка с цифрами; хотя бы одна цифра ("5.", ".5" допустимы,
    ** экспонента — нет). Все цифры подряд образуют модуль, а число цифр
    ** после точки — масштаб.
    **
    ** The xs:decimal lexis: an optional sign, digits, an optional
    ** point with digits; at least one digit ("5.", ".5" are allowed,
    ** an exponent is not). All the digits in a row form the absolute value, and the number of digits
    ** after the point is the scale.
    */
    bool readDecimal(std::string_view text, Decimal& result)
    {
        std::string_view value = trimSpaces(text);
        bool negative = !value.empty() && value.front() == '-';
        std::string digits;
        std::size_t scale = 0;
        bool point = false;

        if (!value.empty() && (value.front() == '-' || value.front() == '+'))
            value.remove_prefix(1);
        for (char symbol : value)
        {
            if (symbol == '.' && !point)
                point = true;
            else if (symbol < '0' || symbol > '9')
                return false;
            else
                digits.push_back(symbol);
            if (point && symbol != '.')
                ++scale;
        }
        if (digits.empty())
            return false;
        result = detail::makeDecimal(negative, digits, scale);
        return true;
    }

    // Модуль через unsigned: так и наименьшее long long не переполняется
    // The absolute value through unsigned: this way even the smallest long long does not overflow
    Decimal decimalFromInteger(long long value)
    {
        unsigned long long magnitude = static_cast<unsigned long long>(value);

        if (value < 0)
            magnitude = 0ULL - magnitude;
        return detail::makeDecimal(value < 0, std::to_string(magnitude), 0);
    }

    // Только целые double до 2^53 переводятся точно (count(), string-length() и т.п.)
    // Only integer doubles up to 2^53 are converted exactly (count(), string-length() and so on)
    bool decimalFromDouble(double value, Decimal& result)
    {
        const double limit = 9007199254740992.0;

        if (!std::isfinite(value) || std::floor(value) != value || std::fabs(value) > limit)
            return false;
        result = decimalFromInteger(static_cast<long long>(value));
        return true;
    }
}
