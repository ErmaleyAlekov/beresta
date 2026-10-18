#include "utils/xml_chars.hpp"
#include "utils/utf8.hpp"
#include "utils/xml_name_table.hpp"

#include <cstdint>

namespace beresta::utils
{
    // Классы ASCII-символов: 2 — начало имени, 1 — только внутри имени, 0 — не символ имени
    // ASCII character classes: 2 — a name start, 1 — only inside a name, 0 — not a name character
    const unsigned char ASCII_NAME_CLASSES[128] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 0x00
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 0x10
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,   // 0x20: '-' '.'
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0,   // 0x30: 0-9 ':'
        0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,   // 0x40: A-O
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 2,   // 0x50: P-Z '_'
        0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,   // 0x60: a-o
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0    // 0x70: p-z
    };

    namespace
    {
        // Диапазон кодов [first, last] из грамматики имён XML 1.0
        // A code range [first, last] from the XML 1.0 name grammar
        struct CodeRange
        {
            std::uint32_t first;
            std::uint32_t last;
        };

        // NameStartChar 5-й редакции (продукция [4]) без ASCII: ASCII проверяется таблицей
        // The 5th edition NameStartChar (production [4]) without ASCII: ASCII is checked by the table
        const CodeRange NAME_START_RANGES[] = {
            {0xC0, 0xD6}, {0xD8, 0xF6}, {0xF8, 0x2FF}, {0x370, 0x37D}, {0x37F, 0x1FFF},
            {0x200C, 0x200D}, {0x2070, 0x218F}, {0x2C00, 0x2FEF}, {0x3001, 0xD7FF},
            {0xF900, 0xFDCF}, {0xFDF0, 0xFFFD}, {0x10000, 0xEFFFF}
        };

        // Дополнительные NameChar (продукция [4a]) вне ASCII
        // Additional NameChars (production [4a]) outside ASCII
        const CodeRange NAME_EXTRA_RANGES[] = {
            {0xB7, 0xB7}, {0x300, 0x36F}, {0x203F, 0x2040}
        };

        // Попадает ли код в один из диапазонов таблицы
        // Whether a code falls into one of the table ranges
        template <std::size_t COUNT>
        bool inRanges(std::uint32_t codePoint, const CodeRange (&ranges)[COUNT]) noexcept
        {
            for (const CodeRange& range : ranges)
            {
                if (codePoint >= range.first && codePoint <= range.last)
                    return true;
            }
            return false;
        }

        // Символ имени вне ASCII — по таблицам диапазонов / A non-ASCII name character — by the range tables
        bool isNameCodePoint(std::uint32_t codePoint, bool first) noexcept
        {
            if (inRanges(codePoint, NAME_START_RANGES))
                return true;
            return !first && inRanges(codePoint, NAME_EXTRA_RANGES);
        }
    }

    /*
    ** Длина в байтах символа имени, начинающегося в text[offset], или 0,
    ** если там не символ имени (или некорректный UTF-8). ASCII решает
    ** таблица, остальное — декодирование UTF-8 и таблицы диапазонов.
    **
    ** The length in bytes of the name character starting at text[offset], or 0
    ** if there is no name character there (or invalid UTF-8). ASCII is decided by
    ** the table, the rest — by UTF-8 decoding and the range tables.
    */
    std::size_t nameCharLength(std::string_view text, std::size_t offset, bool first) noexcept
    {
        std::size_t next = offset;
        std::uint32_t codePoint = 0;

        if (offset >= text.size())
            return 0;
        unsigned char lead = static_cast<unsigned char>(text[offset]);
        if (lead < 0x80)
        {
            unsigned char kind = ASCII_NAME_CLASSES[lead];
            if (kind == NAME_CLASS_START || (kind == NAME_CLASS_INNER && !first))
                return 1;
            return 0;
        }
        if (!decodeUtf8Char(text, next, codePoint) || !isNameCodePoint(codePoint, first))
            return 0;
        return next - offset;
    }

    /*
    ** Длина всего имени, начинающегося в offset (0 — имени там нет). Это
    ** единая точка решения «что такое имя» для сканера, DTD и проверок DOM.
    ** Цикл по ASCII идёт без вызова функции на каждый символ.
    **
    ** The length of the whole name starting at offset (0 — there is no name there).
    ** This is the single point of deciding "what a name is" for the scanner, the DTD
    ** and the DOM checks. The loop over ASCII goes without a function call per character.
    */
    std::size_t nameLength(std::string_view text, std::size_t offset) noexcept
    {
        std::size_t index = offset + nameCharLength(text, offset, true);

        if (index == offset)
            return 0;
        while (index < text.size())
        {
            unsigned char code = static_cast<unsigned char>(text[index]);
            std::size_t step = 1;
            if (code >= 0x80)
                step = nameCharLength(text, index, false);
            else if (ASCII_NAME_CLASSES[code] == NAME_CLASS_NONE)
                step = 0;
            if (step == 0)
                break;
            index += step;
        }
        return index - offset;
    }
}
