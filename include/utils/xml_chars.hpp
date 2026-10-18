#ifndef XML_CHARS_HPP
#define XML_CHARS_HPP

#include <cstddef>
#include <string_view>

namespace beresta::utils
{
    /*
    ** Набор предикатов для классификации символов XML.
    **
    ** Байтовые предикаты (isNameStartChar / isNameChar) точны для ASCII,
    ** а любой байт >= 0x80 считают «возможным началом символа имени»:
    ** они годятся для быстрых развилок («здесь начинается имя?»).
    ** Окончательное решение принимает nameCharLength(): он декодирует
    ** UTF-8 и проверяет код по таблицам NameStartChar / NameChar
    ** XML 1.0 5-й редакции (продукции [4] и [4a]).
    **
    ** A set of predicates for classifying XML characters.
    ** The byte predicates (isNameStartChar / isNameChar) are exact for ASCII,
    ** and treat any byte >= 0x80 as "a possible start of a name character":
    ** they are suitable for fast branching ("does a name start here?").
    ** The final decision is made by nameCharLength(): it decodes
    ** UTF-8 and checks the code against the NameStartChar / NameChar tables
    ** of XML 1.0 5th edition (productions [4] and [4a]).
    */

    // Пробельный символ XML: пробел, таб, перевод строки, возврат каретки
    // An XML whitespace character: space, tab, line feed, carriage return
    bool isWhitespace(char symbol) noexcept;

    // Может ли байт начинать XML-имя (для байтов >= 0x80 — «возможно»)
    // Whether a byte can start an XML name (for bytes >= 0x80 — "possibly")
    bool isNameStartChar(char symbol) noexcept;

    // Может ли байт стоять внутри XML-имени (для байтов >= 0x80 — «возможно»)
    // Whether a byte can be inside an XML name (for bytes >= 0x80 — "possibly")
    bool isNameChar(char symbol) noexcept;

    // Длина в байтах символа имени в позиции offset (first — первый символ имени); 0 — не символ имени
    // The length in bytes of the name character at position offset (first — the first name character); 0 — not a name character
    std::size_t nameCharLength(std::string_view text, std::size_t offset, bool first) noexcept;

    // Длина всего имени с позиции offset в байтах; 0 — имени там нет
    // The length of the whole name from position offset in bytes; 0 — there is no name there
    std::size_t nameLength(std::string_view text, std::size_t offset) noexcept;

    // Является ли строка корректным XML-именем (Name по 5-й редакции)
    // Whether the string is a valid XML name (Name by the 5th edition)
    bool isValidName(std::string_view name) noexcept;

    // Состоит ли строка только из пробельных символов (пустая — тоже да)
    // Whether the string consists only of whitespace (an empty one — yes too)
    bool isWhitespaceOnly(std::string_view text) noexcept;
}

#endif
