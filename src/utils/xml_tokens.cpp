#include "utils/xml_tokens.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::utils
{
    // Токены — максимальные отрезки без пробельных символов
    // Tokens are maximal runs without whitespace characters
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

    // Каждый символ — символ имени (NameChar с учётом Unicode), и токен не пуст
    // Every character is a name character (NameChar taking Unicode into account), and the token is not empty
    bool isValidNmtoken(std::string_view token) noexcept
    {
        std::size_t index = 0;

        if (token.empty())
            return false;
        while (index < token.size())
        {
            std::size_t length = nameCharLength(token, index, false);

            if (length == 0)
                return false;
            index += length;
        }
        return true;
    }

    // Куски между пробелами; несколько пробелов подряд пустых токенов не дают
    // Pieces between spaces; several spaces in a row do not give empty tokens
    std::vector<std::string_view> splitOnSpaces(std::string_view value)
    {
        std::vector<std::string_view> tokens;
        std::size_t position = 0;

        while (position <= value.size())
        {
            std::size_t space = value.find(' ', position);

            if (space == std::string_view::npos)
                space = value.size();
            if (space > position)
                tokens.push_back(value.substr(position, space - position));
            position = space + 1;
        }
        return tokens;
    }

    // PubidChar ::= #x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%]
    bool isValidPublicId(std::string_view text) noexcept
    {
        static constexpr std::string_view PUNCTUATION = "-'()+,./:=?;!*#@$_% \r\n";

        for (char symbol : text)
        {
            bool letter = (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z');
            bool digit = symbol >= '0' && symbol <= '9';

            if (!letter && !digit && PUNCTUATION.find(symbol) == std::string_view::npos)
                return false;
        }
        return true;
    }
}
