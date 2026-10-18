#include "utils/string_utils.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::utils
{
    // Сдвигаем правую границу влево, пока последний символ — пробельный
    // Moves the right bound to the left while the last character is whitespace
    std::string_view trimRight(std::string_view text) noexcept
    {
        std::size_t length = text.size();

        while (length > 0 && isWhitespace(text[length - 1]))
            --length;
        return text.substr(0, length);
    }

    // Сначала сравниваем длины, затем посимвольно в нижнем регистре
    // First the lengths are compared, then character by character in lower case
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
    // An own implementation instead of std::tolower: does not depend on the current locale
    char toLowerAscii(char symbol) noexcept
    {
        if (symbol >= 'A' && symbol <= 'Z')
            return static_cast<char>(symbol - 'A' + 'a');
        return symbol;
    }

    // Куски между \r копируются целиком; и "\r\n", и одиночный "\r" дают один "\n"
    // Pieces between \r are copied whole; both "\r\n" and a lone "\r" give one "\n"
    // Поиск первого символа набора: внешний цикл по тексту, внутренний — по набору
    // Finding the first character of the set: the outer loop over the text, the inner one over the set
    std::size_t findAny(std::string_view text, std::size_t from, std::string_view symbols) noexcept
    {
        for (std::size_t index = from; index < text.size(); ++index)
        {
            char symbol = text[index];
            for (char wanted : symbols)
            {
                if (symbol == wanted)
                    return index;
            }
        }
        return std::string_view::npos;
    }

    std::string normalizeLineEndings(std::string_view text)
    {
        std::string result;
        std::size_t position = 0;

        result.reserve(text.size());
        while (position < text.size())
        {
            std::size_t carriage = text.find('\r', position);

            if (carriage == std::string_view::npos)
                carriage = text.size();
            result.append(text.substr(position, carriage - position));
            if (carriage == text.size())
                break;
            result += '\n';
            position = carriage + 1;
            if (position < text.size() && text[position] == '\n')
                ++position;
        }
        return result;
    }
}
