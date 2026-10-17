#include "utils/xml_tokens.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::utils
{
    // Токены — максимальные отрезки без пробельных символов
    std::vector<std::string_view> splitTokens(std::string_view value)
    {
        std::vector<std::string_view> tokens;
        std::size_t position = 0;

        while (position < value.size())
        {
            while (position < value.size() && isWhitespace(value[position]))
                ++position;
            std::size_t start = position;
            while (position < value.size() && !isWhitespace(value[position]))
                ++position;
            if (position > start)
                tokens.push_back(value.substr(start, position - start));
        }
        return tokens;
    }

    // Каждый символ — символ имени, и токен не пуст
    bool isValidNmtoken(std::string_view token) noexcept
    {
        if (token.empty())
            return false;
        for (char symbol : token)
        {
            if (!isNameChar(symbol))
                return false;
        }
        return true;
    }
}
