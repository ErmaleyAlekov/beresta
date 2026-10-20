#include "suites/test_suites.hpp"
#include "exception/dom_exception.hpp"
#include "utils/utf8.hpp"
#include "xsd/regex/xsd_regex.hpp"

namespace
{
    // Совпадает ли с шаблоном строка из одной кодовой точки
    // Whether a string of one code point matches the pattern
    bool matchesCode(const char* pattern, std::uint32_t code)
    {
        std::string text;

        beresta::utils::appendUtf8(text, code);
        return beresta::xsd::Regex(pattern).matches(text);
    }

    /*
    ** Общие категории Unicode 3.1 целиком, а не только основные письменности:
    ** подкатегории (Lt, Nl, Pd), метки M/Mn/Mc/Me, неназначенные коды Cn
    ** (U+0220 появился лишь в Unicode 3.2) и символы вне BMP (U+1D7A8).
    **
    ** The Unicode 3.1 general categories in full, not only the main scripts:
    ** subcategories (Lt, Nl, Pd), the M/Mn/Mc/Me marks, the unassigned Cn codes
    ** (U+0220 appeared only in Unicode 3.2) and characters outside the BMP (U+1D7A8).
    */
    void testCategories()
    {
        TEST_ASSERT(matchesCode("\\p{Lu}", 0x41));
        TEST_ASSERT(matchesCode("\\p{Lu}", 0x1D7A8));
        TEST_ASSERT(!matchesCode("\\p{Lu}", 0x61));
        TEST_ASSERT(matchesCode("\\p{Lt}", 0x1C5));
        TEST_ASSERT(matchesCode("\\p{Mn}", 0x64B));
        TEST_ASSERT(!matchesCode("\\p{Mn}", 0x903));
        TEST_ASSERT(matchesCode("\\p{Mc}", 0x903));
        TEST_ASSERT(matchesCode("\\p{Me}", 0x20DD));
        TEST_ASSERT(matchesCode("\\p{M}", 0x1D172));
        TEST_ASSERT(matchesCode("\\p{Nl}", 0x3025));
        TEST_ASSERT(!matchesCode("\\p{Nl}", 0xB2));
        TEST_ASSERT(matchesCode("\\p{Pd}", 0x301C));
        TEST_ASSERT(matchesCode("\\p{Cn}", 0x220));
        TEST_ASSERT(!matchesCode("\\p{Cn}", 0x41));
        TEST_ASSERT(matchesCode("\\p{C}", 0x220));
        TEST_ASSERT(matchesCode("\\p{Co}", 0xE000));
        TEST_ASSERT(matchesCode("\\P{Lm}", 0x5D0));
        TEST_ASSERT(!matchesCode("\\P{Lm}", 0x2B0));
    }

    /*
    ** Блоки \p{IsИмя} из таблицы Unicode 3.1, включая блоки вне BMP
    ** (IsOldItalic) и составные (IsSpecials — два диапазона). IsPrivateUse —
    ** только E000-F8FF, как в таблице XSD 1.0 2-й редакции. Неизвестные
    ** имена блоков и категорий, Cs и имена не в том регистре — ошибки.
    **
    ** The \p{IsName} blocks from the Unicode 3.1 table, including the blocks outside the BMP
    ** (IsOldItalic) and compound ones (IsSpecials — two ranges). IsPrivateUse is
    ** only E000-F8FF, as in the XSD 1.0 2nd edition table. Unknown
    ** block and category names, Cs and names in the wrong case are errors.
    */
    void testBlocks()
    {
        TEST_ASSERT(matchesCode("\\p{IsBasicLatin}", 0x41));
        TEST_ASSERT(!matchesCode("\\p{IsBasicLatin}", 0x42F));
        TEST_ASSERT(matchesCode("\\p{IsCyrillic}", 0x42F));
        TEST_ASSERT(matchesCode("\\P{IsCyrillic}", 0x41));
        TEST_ASSERT(matchesCode("\\p{IsCJKUnifiedIdeographsExtensionA}", 0x3400));
        TEST_ASSERT(matchesCode("\\p{IsOldItalic}", 0x10300));
        TEST_ASSERT(matchesCode("\\p{IsSpecials}", 0xFEFF));
        TEST_ASSERT(matchesCode("\\p{IsSpecials}", 0xFFFD));
        TEST_ASSERT(matchesCode("\\p{IsPrivateUse}", 0xE000));
        TEST_ASSERT(!matchesCode("\\p{IsPrivateUse}", 0xF0000));
        TEST_ASSERT_THROWS(beresta::xsd::Regex("\\p{IsCyrillicSupplement}"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("\\p{Is}"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("\\p{Cs}"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("\\p{Lx}"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("\\p{lu}"), beresta::DomException);
    }

    /*
    ** Многосимвольные экранирования вне ASCII: \d — все цифры Nd (деванагари,
    ** математические), \w исключает форматирующие символы Cf, \i и \c следуют
    ** приложению B XML 1.0 (U+3007 — буква, U+00AA — нет, U+20DD исключён),
    ** и внутри класса [\d] / [\i] значат то же самое.
    **
    ** Multi-character escapes outside ASCII: \d is all Nd digits (Devanagari,
    ** mathematical), \w excludes the Cf format characters, \i and \c follow
    ** XML 1.0 appendix B (U+3007 is a letter, U+00AA is not, U+20DD is excluded),
    ** and inside a class [\d] / [\i] mean the same.
    */
    void testMultiCharEscapes()
    {
        TEST_ASSERT(matchesCode("\\d", 0x966));
        TEST_ASSERT(matchesCode("\\d", 0x1D7CE));
        TEST_ASSERT(!matchesCode("\\D", 0x966));
        TEST_ASSERT(!matchesCode("\\w", 0x70F));
        TEST_ASSERT(matchesCode("\\w", 0x2044));
        TEST_ASSERT(matchesCode("\\i", 0x3007));
        TEST_ASSERT(!matchesCode("\\i", 0xAA));
        TEST_ASSERT(matchesCode("[\\i]", 0x212E));
        TEST_ASSERT(matchesCode("\\c", 0xB7));
        TEST_ASSERT(!matchesCode("\\c", 0x20DD));
        TEST_ASSERT(matchesCode("[\\d]", 0x9E6));
    }
}

namespace suites
{
    // Регистрация тестов свойств Unicode в регулярных выражениях XSD
    // Registers the tests of Unicode properties in XSD regular expressions
    void registerXsdRegexUnicodeTests(test::TestRunner& runner)
    {
        runner.add("XSD regex: категории Unicode", testCategories);
        runner.add("XSD regex: блоки Unicode", testBlocks);
        runner.add("XSD regex: \\d \\w \\i \\c вне ASCII", testMultiCharEscapes);
    }
}
