#include "utils/xml_chars.hpp"

namespace beresta::utils
{
    // Проверяет четыре пробельных символа, определённых стандартом XML
    // Checks the four whitespace characters defined by the XML standard
    bool isWhitespace(char symbol) noexcept
    {
        if (symbol == ' ' || symbol == '\t')
            return true;
        return symbol == '\n' || symbol == '\r';
    }

    /*
    ** Первый символ имени: латинская буква, '_', ':' или любой
    ** не-ASCII байт (начало многобайтового символа UTF-8).
    **
    ** The first character of a name: a Latin letter, '_', ':' or any
    ** non-ASCII byte (the start of a multi-byte UTF-8 character).
    */
    bool isNameStartChar(char symbol) noexcept
    {
        unsigned char code = static_cast<unsigned char>(symbol);

        if (code >= 0x80)
            return true;
        if ((code >= 'a' && code <= 'z') || (code >= 'A' && code <= 'Z'))
            return true;
        return code == '_' || code == ':';
    }

    // Внутри имени дополнительно разрешены цифры, '-' и '.'
    // Inside a name digits, '-' and '.' are allowed additionally
    bool isNameChar(char symbol) noexcept
    {
        if (isNameStartChar(symbol))
            return true;
        if (symbol >= '0' && symbol <= '9')
            return true;
        return symbol == '-' || symbol == '.';
    }

    // Имя непустое: первый символ — NameStartChar, остальные — NameChar (с учётом Unicode)
    // The name is non-empty: the first character is a NameStartChar, the rest are NameChars (taking Unicode into account)
    bool isValidName(std::string_view name) noexcept
    {
        return !name.empty() && nameLength(name, 0) == name.size();
    }

    // Пробегаем строку и ищем первый непробельный символ
    // Runs over the string looking for the first non-whitespace character
    bool isWhitespaceOnly(std::string_view text) noexcept
    {
        for (char symbol : text)
        {
            if (!isWhitespace(symbol))
                return false;
        }
        return true;
    }
}
