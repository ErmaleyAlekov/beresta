#include "utils/utf8.hpp"

namespace beresta::utils
{
    // Символ начинается с любого байта, кроме продолжения вида 10xxxxxx
    // A character starts at any byte except a continuation of the form 10xxxxxx
    std::size_t countCodePoints(std::string_view text) noexcept
    {
        std::size_t count = 0;

        for (char symbol : text)
        {
            if ((static_cast<unsigned char>(symbol) & 0xC0) != 0x80)
                ++count;
        }
        return count;
    }

    // Первый символ строки; используется для проверки начала имён и шаблонов
    // The first character of a string; used to check the start of names and patterns
    std::uint32_t firstCodePoint(std::string_view text) noexcept
    {
        std::size_t index = 0;
        std::uint32_t codePoint = 0;

        if (text.empty() || !decodeUtf8Char(text, index, codePoint))
            return 0;
        return codePoint;
    }
}
