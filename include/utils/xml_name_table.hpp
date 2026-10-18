#ifndef BERESTA_XML_NAME_TABLE_HPP
#define BERESTA_XML_NAME_TABLE_HPP

namespace beresta::utils
{
    // Класс ASCII-символа в XML-имени: 0 — не символ имени / The class of an ASCII character in an XML name: 0 — not a name character
    constexpr unsigned char NAME_CLASS_NONE = 0;

    // Символ допустим только внутри имени (цифры, '-', '.') / The character is allowed only inside a name (digits, '-', '.')
    constexpr unsigned char NAME_CLASS_INNER = 1;

    // Символ может начинать имя (буквы, '_', ':') / The character can start a name (letters, '_', ':')
    constexpr unsigned char NAME_CLASS_START = 2;

    /*
    ** Таблица классов для байтов 0x00-0x7F (xml_name_chars.cpp). Почти все
    ** имена в реальных документах — ASCII, и таблица отвечает на вопрос
    ** «символ имени?» одним обращением к памяти, без декодирования UTF-8.
    **
    ** The class table for the bytes 0x00-0x7F (xml_name_chars.cpp). Almost all
    ** names in real documents are ASCII, and the table answers "a name character?"
    ** with one memory access, without UTF-8 decoding.
    */
    extern const unsigned char ASCII_NAME_CLASSES[128];
}

#endif
