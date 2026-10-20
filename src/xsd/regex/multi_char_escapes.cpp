#include "xsd/regex/char_set.hpp"
#include "xsd/regex/unicode_tables.hpp"

namespace beresta::xsd
{
    // Добавление всех диапазонов константной таблицы
    // Adds all ranges of a constant table
    static void addRanges(CharSet& result, const UnicodeRange* ranges, std::size_t count)
    {
        for (std::size_t index = 0; index < count; ++index)
            result.add(ranges[index].first, ranges[index].last);
    }

    /*
    ** \i — начало имени XML (Letter | '_' | ':'), \c — символ имени (NameChar).
    ** XSD 1.0 определяет их через классы приложения B XML 1.0, поэтому здесь
    ** не годятся ни категории Unicode, ни таблицы имён 5-й редакции XML.
    **
    ** \i is the start of an XML name (Letter | '_' | ':'), \c is a name character (NameChar).
    ** XSD 1.0 defines them through the XML 1.0 appendix B classes, so neither
    ** the Unicode categories nor the XML 5th edition name tables fit here.
    */
    static CharSet nameCharSet(bool fullName)
    {
        CharSet result;

        addRanges(result, XML_NAME_START_RANGES, XML_NAME_START_COUNT);
        if (fullName)
            addRanges(result, XML_NAME_EXTRA_RANGES, XML_NAME_EXTRA_COUNT);
        result.normalize();
        return result;
    }

    /*
    ** \s — пробельные символы XML; \d — десятичные цифры (\p{Nd});
    ** \i и \c — символы имён; \w — всё, кроме знаков препинания,
    ** разделителей и «прочих» (\p{P}, \p{Z}, \p{C}), как в спецификации XSD.
    **
    ** \s — XML whitespace; \d — decimal digits (\p{Nd});
    ** \i and \c — name characters; \w — everything except punctuation,
    ** separators and "others" (\p{P}, \p{Z}, \p{C}), as in the XSD specification.
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
