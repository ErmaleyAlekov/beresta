#ifndef XML_CHARS_HPP
#define XML_CHARS_HPP

#include <string_view>

namespace beresta::utils
{
    /*
    ** Набор предикатов для классификации символов XML.
    ** Работаем побайтово: все байты >= 0x80 (части многобайтовых
    ** последовательностей UTF-8) считаются допустимыми символами имени.
    ** Это осознанное упрощение спецификации XML 1.0 (KISS): кириллические
    ** и прочие не-ASCII имена работают, а таблицы Unicode не нужны.
    */

    // Пробельный символ XML: пробел, таб, перевод строки, возврат каретки
    bool isWhitespace(char symbol) noexcept;

    // Может ли символ стоять первым в XML-имени
    bool isNameStartChar(char symbol) noexcept;

    // Может ли символ стоять внутри XML-имени (после первого)
    bool isNameChar(char symbol) noexcept;

    // Является ли строка корректным XML-именем
    bool isValidName(std::string_view name) noexcept;

    // Состоит ли строка только из пробельных символов (пустая — тоже да)
    bool isWhitespaceOnly(std::string_view text) noexcept;
}

#endif
