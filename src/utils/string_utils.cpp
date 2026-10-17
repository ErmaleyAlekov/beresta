#include "utils/string_utils.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::utils
{
    // Сдвигаем правую границу влево, пока последний символ — пробельный
    std::string_view trimRight(std::string_view text) noexcept
    {
        std::size_t length = text.size();

        while (length > 0 && isWhitespace(text[length - 1]))
            --length;
        return text.substr(0, length);
    }

    // Сначала сравниваем длины, затем посимвольно в нижнем регистре
    bool equalsIgnoreCaseAscii(std::string_view left, std::string_view right) noexcept
    {
        if (left.size() != right.size())
            return false;
        for (std::size_t index = 0; index < left.size(); ++index)
        {
            if (toLowerAscii(left[index]) != toLowerAscii(right[index]))
                return false;
        }
        return true;
    }

    // Собственная реализация вместо std::tolower: не зависит от текущей локали
    char toLowerAscii(char symbol) noexcept
    {
        if (symbol >= 'A' && symbol <= 'Z')
            return static_cast<char>(symbol - 'A' + 'a');
        return symbol;
    }
}
