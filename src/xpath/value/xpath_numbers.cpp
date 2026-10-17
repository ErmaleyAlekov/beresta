#include "xpath/xpath_value.hpp"
#include "utils/xml_chars.hpp"

#include <cmath>

namespace beresta::xpath
{
    /*
    ** Цифры с необязательной точкой: "12", "12.", "12.5", ".5".
    ** Возвращает false, если встретился посторонний символ или цифр нет.
    */
    static bool parseDecimal(std::string_view text, long double& result) noexcept
    {
        long double scale = 1.0L;
        bool digits = false;
        bool fraction = false;

        result = 0.0L;
        for (char symbol : text)
        {
            if (symbol == '.' && !fraction)
            {
                fraction = true;
                continue;
            }
            if (symbol < '0' || symbol > '9')
                return false;
            digits = true;
            if (fraction)
                scale /= 10.0L;
            result = result * 10.0L + (symbol - '0');
        }
        result *= scale;
        return digits;
    }

    /*
    ** Собственный разбор (std::strtod зависит от локали и принимает
    ** лишние формы вроде "1e5" или "0x10", которые XPath не допускает).
    */
    double stringToNumber(std::string_view text) noexcept
    {
        std::size_t begin = 0;
        std::size_t end = text.size();
        long double result = 0.0L;

        while (begin < end && utils::isWhitespace(text[begin]))
            ++begin;
        while (end > begin && utils::isWhitespace(text[end - 1]))
            --end;
        bool negative = begin < end && text[begin] == '-';
        if (negative)
            ++begin;
        if (!parseDecimal(text.substr(begin, end - begin), result))
            return std::nan("");
        if (negative)
            return -static_cast<double>(result);
        return static_cast<double>(result);
    }
}
