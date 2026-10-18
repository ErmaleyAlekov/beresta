#include "suites/test_suites.hpp"
#include "support/parse_helpers.hpp"

#include <string>

/*
** Регрессионный тест расхождения с набором W3C (hst-lhs-007, hst-lhs-008):
** кодировка, определённая по BOM, обязана совпадать с encoding декларации.
**
** A regression test for a deviation from the W3C suite (hst-lhs-007, hst-lhs-008):
** the encoding detected by the BOM must match the encoding of the declaration.
*/
namespace
{
    // Текст ASCII в UTF-16LE с BOM (каждый символ — два байта, младший первым)
    // ASCII text in UTF-16LE with a BOM (each character is two bytes, the low one first)
    std::string utf16le(const std::string& ascii)
    {
        std::string result = "\xFF\xFE";

        for (char symbol : ascii)
        {
            result += symbol;
            result += '\0';
        }
        return result;
    }

    // BOM UTF-8 + iso-8859-1 и BOM UTF-16 + utf-8 — фатальная ошибка; согласованные пары — нет
    // A UTF-8 BOM + iso-8859-1 and a UTF-16 BOM + utf-8 are a fatal error; matching pairs are not
    void testBomMatchesDeclaration()
    {
        std::string utf8Bom = "\xEF\xBB\xBF";
        beresta::ParserOptions forced;

        TEST_ASSERT(support::errorCodeOf(utf8Bom + "<?xml version='1.0' encoding='iso-8859-1'?><x/>")
            == beresta::DomErrorCode::ENCODING_ERROR);
        TEST_ASSERT(support::errorCodeOf(utf16le("<?xml version='1.0' encoding='utf-8'?><x/>"))
            == beresta::DomErrorCode::ENCODING_ERROR);
        TEST_ASSERT(support::errorCodeOf(utf8Bom + "<?xml version='1.0' encoding='UTF-8'?><x/>")
            == beresta::DomErrorCode::NOT_FOUND);
        TEST_ASSERT(support::errorCodeOf(utf16le("<?xml version='1.0' encoding='UTF-16'?><x/>"))
            == beresta::DomErrorCode::NOT_FOUND);
        forced.forcedEncoding = "UTF-8";
        TEST_ASSERT(beresta::XmlParser(forced).parse(utf8Bom + "<?xml version='1.0' encoding='koi8-r'?><x/>")
            .getRoot() != nullptr);
    }
}

namespace suites
{
    // Регистрация теста согласия BOM и декларации
    // Registers the test of the BOM and declaration agreement
    void registerW3cEncodingTests(test::TestRunner& runner)
    {
        runner.add("W3C: BOM и encoding декларации согласованы", testBomMatchesDeclaration);
    }
}
