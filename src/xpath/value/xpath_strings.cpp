#include "xpath/xpath_value.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace beresta::xpath
{
    // Атрибут — его значение; остальные узлы — текстовое содержимое (у PI — данные)
    // An attribute — its value; other nodes — the text content (for a PI — the data)
    std::string stringValue(const XPathItem& item)
    {
        if (item.attribute != nullptr)
            return item.attribute->value;
        if (item.node == nullptr)
            return std::string();
        return item.node->getTextContent();
    }

    // Ставит десятичную точку после point цифр, дополняя нулями слева или справа
    // Puts the decimal point after point digits, padding with zeros on the left or on the right
    static std::string placeDecimalPoint(std::string digits, long point)
    {
        while (point > static_cast<long>(digits.size()))
            digits += '0';
        while (point <= 0)
        {
            digits.insert(0, "0");
            ++point;
        }
        if (point < static_cast<long>(digits.size()))
            digits.insert(static_cast<std::size_t>(point), ".");
        return digits;
    }

    /*
    ** Экспоненциальная запись "1.5e+20" переписывается в обычную:
    ** XPath запрещает экспоненту в строковом представлении числа.
    **
    ** The exponential form "1.5e+20" is rewritten into the ordinary one:
    ** XPath forbids an exponent in the string representation of a number.
    */
    static std::string expandExponent(const std::string& text)
    {
        std::size_t marker = text.find('e');

        if (marker == std::string::npos)
            return text;
        std::string digits = text.substr(0, marker);
        long exponent = std::strtol(text.c_str() + marker + 1, nullptr, 10);
        bool negative = !digits.empty() && digits[0] == '-';
        if (negative)
            digits.erase(0, 1);
        std::size_t dot = digits.find('.');
        if (dot == std::string::npos)
            dot = digits.size();
        else
            digits.erase(dot, 1);
        std::string result = placeDecimalPoint(digits, static_cast<long>(dot) + exponent);
        if (negative)
            return "-" + result;
        return result;
    }

    /*
    ** Целые печатаются без дробной части, остальные — с минимально
    ** достаточной точностью (15 значащих цифр, при потере точности — 17).
    ** Запятая (возможна при чужой локали) заменяется точкой.
    **
    ** Integers are printed without a fraction, the others with the minimally
    ** sufficient precision (15 significant digits, 17 if precision would be lost).
    ** A comma (possible with a foreign locale) is replaced with a point.
    */
    std::string numberToString(double value)
    {
        char buffer[64];

        if (std::isnan(value))
            return "NaN";
        if (std::isinf(value) && value > 0)
            return "Infinity";
        if (std::isinf(value))
            return "-Infinity";
        if (value == std::floor(value) && std::fabs(value) < 1e15)
            return std::to_string(static_cast<long long>(value));
        std::snprintf(buffer, sizeof(buffer), "%.15g", value);
        if (std::strtod(buffer, nullptr) != value)
            std::snprintf(buffer, sizeof(buffer), "%.17g", value);
        std::string text(buffer);
        for (char& symbol : text)
        {
            if (symbol == ',')
                symbol = '.';
        }
        return expandExponent(text);
    }
}
