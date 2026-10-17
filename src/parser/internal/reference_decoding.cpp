#include "parser/internal/reference_decoding.hpp"
#include "utils/utf8.hpp"
#include "utils/xml_chars.hpp"

#include <cstdint>

namespace beresta::detail
{
    // Пять сущностей, которые XML знает без всякого DTD
    std::optional<char> predefinedEntity(std::string_view name) noexcept
    {
        if (name == "lt")
            return '<';
        if (name == "gt")
            return '>';
        if (name == "amp")
            return '&';
        if (name == "quot")
            return '"';
        if (name == "apos")
            return '\'';
        return std::nullopt;
    }

    // Значение цифры в системе base или base, если символ — не цифра этой системы
    static std::uint32_t digitValue(char symbol, std::uint32_t base) noexcept
    {
        if (symbol >= '0' && symbol <= '9')
            return static_cast<std::uint32_t>(symbol - '0');
        if (base == 16 && symbol >= 'a' && symbol <= 'f')
            return static_cast<std::uint32_t>(symbol - 'a' + 10);
        if (base == 16 && symbol >= 'A' && symbol <= 'F')
            return static_cast<std::uint32_t>(symbol - 'A' + 10);
        return base;
    }

    /*
    ** Собственный разбор числа (без std::stoul — он принимает пробелы,
    ** знаки и бросает чужие исключения). Диапазон проверяется после
    ** каждой цифры, поэтому переполнение невозможно.
    */
    bool decodeCharacterReference(std::string_view body, std::string& output)
    {
        std::uint32_t base = 10;
        std::uint32_t value = 0;

        if (!body.empty() && body[0] == 'x')
        {
            base = 16;
            body.remove_prefix(1);
        }
        if (body.empty())
            return false;
        for (char symbol : body)
        {
            std::uint32_t digit = digitValue(symbol, base);
            if (digit >= base)
                return false;
            value = value * base + digit;
            if (value > 0x10FFFF)
                return false;
        }
        if (!utils::isValidXmlCodePoint(value))
            return false;
        utils::appendUtf8(output, value);
        return true;
    }

    // Текстовая декларация — "<?xml" + пробел ... "?>" в самом начале текста
    std::string_view stripTextDeclaration(std::string_view text) noexcept
    {
        if (text.size() < 6 || text.substr(0, 5) != "<?xml" || !utils::isWhitespace(text[5]))
            return text;
        std::size_t end = text.find("?>");
        if (end == std::string_view::npos)
            return text;
        return text.substr(end + 2);
    }
}
