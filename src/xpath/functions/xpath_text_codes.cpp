#include "xpath/internal/xpath_functions.hpp"
#include "utils/utf8.hpp"

namespace beresta::xpath::detail
{
    // Строка -> кодовые точки (символы Unicode), чтобы считать позиции по символам
    // A string -> code points (Unicode characters) to count positions by characters
    std::vector<std::uint32_t> toCodePoints(const std::string& text)
    {
        std::vector<std::uint32_t> codes;
        std::size_t index = 0;
        std::uint32_t code = 0;

        while (index < text.size())
        {
            if (!utils::decodeUtf8Char(text, index, code))
            {
                code = static_cast<unsigned char>(text[index]);
                ++index;
            }
            codes.push_back(code);
        }
        return codes;
    }

    // Кодовые точки [begin, end) -> строка UTF-8
    // Code points [begin, end) -> a UTF-8 string
    std::string fromCodePoints(const std::vector<std::uint32_t>& codes, std::size_t begin,
        std::size_t end)
    {
        std::string text;

        for (std::size_t index = begin; index < end && index < codes.size(); ++index)
            utils::appendUtf8(text, codes[index]);
        return text;
    }
}
