#include "xsd/regex/char_set.hpp"

#include <cstddef>

namespace beresta::xsd
{
    // Именованный набор диапазонов (категория или блок Unicode)
    struct PropertyRanges
    {
        const char* name;
        std::uint32_t ranges[24][2];
        std::size_t count;
    };

    /*
    ** Категории Unicode — упрощённые таблицы для распространённых
    ** письменностей (латиница, кириллица, греческий, иврит, арабский, CJK).
    ** Полные таблицы Unicode занимают сотни килобайт; для фасетов pattern
    ** реальных схем этого покрытия достаточно (ограничение задокументировано).
    */
    static const PropertyRanges PROPERTIES[] = {
        {"Lu", {{0x41, 0x5A}, {0xC0, 0xD6}, {0xD8, 0xDE}, {0x391, 0x3A9}, {0x400, 0x42F}}, 5},
        {"Ll", {{0x61, 0x7A}, {0xDF, 0xF6}, {0xF8, 0xFF}, {0x3B1, 0x3C9}, {0x430, 0x45F}}, 5},
        {"L", {{0x41, 0x5A}, {0x61, 0x7A}, {0xAA, 0xAA}, {0xB5, 0xB5}, {0xBA, 0xBA}, {0xC0, 0xD6},
            {0xD8, 0xF6}, {0xF8, 0x24F}, {0x370, 0x3FF}, {0x400, 0x481}, {0x48A, 0x52F},
            {0x5D0, 0x5EA}, {0x620, 0x64A}, {0x3040, 0x30FF}, {0x4E00, 0x9FFF}, {0xAC00, 0xD7A3}}, 16},
        {"Nd", {{0x30, 0x39}, {0x660, 0x669}, {0x6F0, 0x6F9}, {0x966, 0x96F}, {0xFF10, 0xFF19}}, 5},
        {"N", {{0x30, 0x39}, {0xB2, 0xB3}, {0xB9, 0xB9}, {0xBC, 0xBE}, {0x660, 0x669},
            {0x6F0, 0x6F9}, {0x966, 0x96F}, {0x2150, 0x2189}, {0xFF10, 0xFF19}}, 9},
        {"P", {{0x21, 0x23}, {0x25, 0x2A}, {0x2C, 0x2F}, {0x3A, 0x3B}, {0x3F, 0x40}, {0x5B, 0x5D},
            {0x5F, 0x5F}, {0x7B, 0x7B}, {0x7D, 0x7D}, {0xA1, 0xA1}, {0xA7, 0xA7}, {0xAB, 0xAB},
            {0xB6, 0xB7}, {0xBB, 0xBB}, {0xBF, 0xBF}, {0x2010, 0x2027}, {0x2030, 0x205E},
            {0x3001, 0x3003}}, 18},
        {"S", {{0x24, 0x24}, {0x2B, 0x2B}, {0x3C, 0x3E}, {0x5E, 0x5E}, {0x60, 0x60}, {0x7C, 0x7C},
            {0x7E, 0x7E}, {0xA2, 0xA6}, {0xA8, 0xA9}, {0xAC, 0xAC}, {0xAE, 0xB1}, {0xB4, 0xB4},
            {0xD7, 0xD7}, {0xF7, 0xF7}, {0x20A0, 0x20BF}, {0x2100, 0x214F}, {0x2190, 0x23FF}}, 17},
        {"Z", {{0x20, 0x20}, {0xA0, 0xA0}, {0x1680, 0x1680}, {0x2000, 0x200A}, {0x2028, 0x2029},
            {0x202F, 0x202F}, {0x205F, 0x205F}, {0x3000, 0x3000}}, 8},
        {"C", {{0x00, 0x1F}, {0x7F, 0x9F}, {0xAD, 0xAD}, {0xE000, 0xF8FF}}, 4},
        {"IsBasicLatin", {{0x00, 0x7F}}, 1},
        {"IsLatin-1Supplement", {{0x80, 0xFF}}, 1},
        {"IsLatinExtended-A", {{0x100, 0x17F}}, 1},
        {"IsLatinExtended-B", {{0x180, 0x24F}}, 1},
        {"IsGreek", {{0x370, 0x3FF}}, 1},
        {"IsCyrillic", {{0x400, 0x4FF}}, 1},
        {"IsCyrillicSupplement", {{0x500, 0x52F}}, 1},
        {"IsHebrew", {{0x590, 0x5FF}}, 1},
        {"IsArabic", {{0x600, 0x6FF}}, 1},
        {"IsGeneralPunctuation", {{0x2000, 0x206F}}, 1},
        {"IsHiragana", {{0x3040, 0x309F}}, 1},
        {"IsKatakana", {{0x30A0, 0x30FF}}, 1},
        {"IsCJKUnifiedIdeographs", {{0x4E00, 0x9FFF}}, 1}
    };

    /*
    ** Имя свойства: точное совпадение или однобуквенная категория для
    ** подкатегорий без собственной таблицы (\p{Lo} -> L, \p{Pd} -> P).
    */
    CharSet unicodePropertySet(std::string_view name, bool& known)
    {
        CharSet result;
        std::string_view lookup = name;

        known = false;
        for (int pass = 0; pass < 2 && !known; ++pass)
        {
            for (const PropertyRanges& property : PROPERTIES)
            {
                if (lookup != property.name)
                    continue;
                for (std::size_t index = 0; index < property.count; ++index)
                    result.add(property.ranges[index][0], property.ranges[index][1]);
                known = true;
            }
            if (name.size() == 2 && name.substr(0, 2) != "Is")
                lookup = name.substr(0, 1);
        }
        result.normalize();
        return result;
    }

    // \i — начало XML-имени (буквы, '_', ':'); \c — символ имени (ещё цифры, '-', '.', диакритика)
    static CharSet nameCharSet(bool fullName)
    {
        CharSet result;
        bool known = false;

        result.addSet(unicodePropertySet("L", known));
        result.add('_', '_');
        result.add(':', ':');
        if (fullName)
        {
            result.addSet(unicodePropertySet("Nd", known));
            result.add('-', '.');
            result.add(0xB7, 0xB7);
            result.add(0x300, 0x36F);
        }
        result.normalize();
        return result;
    }

    /*
    ** \s — пробельные символы XML; \d — цифры (Nd); \i и \c — символы имён;
    ** \w — всё, кроме знаков препинания, разделителей и управляющих символов
    ** (определение спецификации XSD).
    */
    CharSet multiCharEscapeSet(char letter)
    {
        CharSet result;
        bool known = false;

        if (letter == 'd')
            return unicodePropertySet("Nd", known);
        if (letter == 'i' || letter == 'c')
            return nameCharSet(letter == 'c');
        if (letter == 's')
        {
            result.add(0x20, 0x20);
            result.add(0x09, 0x0A);
            result.add(0x0D, 0x0D);
            result.normalize();
            return result;
        }
        result.addSet(unicodePropertySet("P", known));
        result.addSet(unicodePropertySet("Z", known));
        result.addSet(unicodePropertySet("C", known));
        return result.complement();
    }
}
