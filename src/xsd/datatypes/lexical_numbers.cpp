#include "xsd/internal/datatype_checks.hpp"

namespace beresta::xsd::detail
{
    // boolean: true | false | 1 | 0
    bool isBooleanLexical(std::string_view value) noexcept
    {
        return value == "true" || value == "false" || value == "1" || value == "0";
    }

    // decimal: [+-]?(цифры(.цифры?)? | .цифры); для integer точка запрещена
    bool isDecimalLexical(std::string_view value, bool integerOnly) noexcept
    {
        std::size_t position = 0;
        bool digits = false;
        bool point = false;

        if (!value.empty() && (value[0] == '+' || value[0] == '-'))
            position = 1;
        for (; position < value.size(); ++position)
        {
            char symbol = value[position];
            if (symbol == '.' && !point && !integerOnly)
                point = true;
            else if (symbol >= '0' && symbol <= '9')
                digits = true;
            else
                return false;
        }
        return digits;
    }

    // float / double: INF, -INF, NaN или десятичная мантисса с необязательной экспонентой
    bool isFloatLexical(std::string_view value) noexcept
    {
        if (value == "INF" || value == "-INF" || value == "NaN")
            return true;
        std::size_t exponent = value.find_first_of("eE");
        if (exponent == std::string_view::npos)
            return isDecimalLexical(value, false);
        return isDecimalLexical(value.substr(0, exponent), false)
            && isDecimalLexical(value.substr(exponent + 1), true);
    }

    // hexBinary: чётное число шестнадцатеричных цифр; длина — в байтах
    std::optional<std::size_t> hexBinaryLength(std::string_view value) noexcept
    {
        if (value.size() % 2 != 0)
            return std::nullopt;
        for (char symbol : value)
        {
            bool hex = (symbol >= '0' && symbol <= '9') || (symbol >= 'a' && symbol <= 'f')
                || (symbol >= 'A' && symbol <= 'F');
            if (!hex)
                return std::nullopt;
        }
        return value.size() / 2;
    }

    /*
    ** base64Binary: символы A-Z a-z 0-9 + / (пробелы игнорируются), длина
    ** кратна 4, '=' — только в конце и не больше двух. Длина — в байтах.
    */
    std::optional<std::size_t> base64BinaryLength(std::string_view value) noexcept
    {
        std::size_t count = 0;
        std::size_t padding = 0;

        for (char symbol : value)
        {
            if (symbol == ' ')
                continue;
            bool letter = (symbol >= 'A' && symbol <= 'Z') || (symbol >= 'a' && symbol <= 'z');
            bool valid = letter || (symbol >= '0' && symbol <= '9') || symbol == '+' || symbol == '/';
            if (symbol == '=')
                ++padding;
            else if (!valid || padding > 0)
                return std::nullopt;
            ++count;
        }
        if (count % 4 != 0 || padding > 2)
            return std::nullopt;
        return count / 4 * 3 - padding;
    }
}
