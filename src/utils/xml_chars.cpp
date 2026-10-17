#include "utils/xml_chars.hpp"

namespace beresta::utils
{
    // Проверяет четыре пробельных символа, определённых стандартом XML
    bool isWhitespace(char symbol) noexcept
    {
        if (symbol == ' ' || symbol == '\t')
            return true;
        return symbol == '\n' || symbol == '\r';
    }

    /*
    ** Первый символ имени: латинская буква, '_', ':' или любой
    ** не-ASCII байт (начало многобайтового символа UTF-8).
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
    bool isNameChar(char symbol) noexcept
    {
        if (isNameStartChar(symbol))
            return true;
        if (symbol >= '0' && symbol <= '9')
            return true;
        return symbol == '-' || symbol == '.';
    }

    // Имя непустое, начинается с допустимого символа и далее состоит из символов имени
    bool isValidName(std::string_view name) noexcept
    {
        if (name.empty() || !isNameStartChar(name[0]))
            return false;
        for (std::size_t index = 1; index < name.size(); ++index)
        {
            if (!isNameChar(name[index]))
                return false;
        }
        return true;
    }

    // Пробегаем строку и ищем первый непробельный символ
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
