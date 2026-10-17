#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "support/parse_helpers.hpp"

#include <string>

namespace
{
    // windows-1251 и KOI8-R по атрибуту encoding: «Привет» в разных байтах
    void testCyrillicSingleByte()
    {
        std::string cp1251 = "<?xml version=\"1.0\" encoding=\"windows-1251\"?><t>\xCF\xF0\xE8\xE2\xE5\xF2</t>";
        std::string koi8 = "<?xml version=\"1.0\" encoding=\"KOI8-R\"?><t>\xF0\xD2\xC9\xD7\xC5\xD4</t>";
        beresta::Document first = beresta::XmlParser().parse(cp1251);
        beresta::Document second = beresta::XmlParser().parse(koi8);

        TEST_ASSERT_EQ(first.getRoot()->getTextContent(), "Привет");
        TEST_ASSERT_EQ(second.getRoot()->getTextContent(), "Привет");
        TEST_ASSERT_EQ(first.getDeclaration()->encoding, "windows-1251");
    }

    // UTF-16LE с BOM, включая символ вне BMP (суррогатная пара)
    void testUtf16()
    {
        std::string utf16("\xFF\xFE<\0a\0>\0\x3D\xD8\x00\xDE<\0/\0a\0>\0", 20);
        beresta::Document document = beresta::XmlParser().parse(utf16);

        TEST_ASSERT_EQ(document.getRoot()->getTextContent(), "\xF0\x9F\x98\x80");
    }

    // Принудительная кодировка перекрывает декларацию
    void testForcedEncoding()
    {
        beresta::ParserOptions options;
        options.forcedEncoding = "cp866";
        std::string bytes = "<t>\x8F\xE0\xA8\xA2\xA5\xE2</t>";

        TEST_ASSERT_EQ(beresta::XmlParser(options).parse(bytes).getRoot()->getTextContent(), "Привет");
    }

    // Битый UTF-8, управляющий символ и неизвестная кодировка — ENCODING_ERROR
    void testEncodingErrors()
    {
        TEST_ASSERT(support::errorCodeOf("<a>\xC3</a>") == beresta::DomErrorCode::ENCODING_ERROR);
        TEST_ASSERT(support::errorCodeOf("<a>\x01</a>") == beresta::DomErrorCode::ENCODING_ERROR);
        TEST_ASSERT(support::errorCodeOf("<?xml version=\"1.0\" encoding=\"x-unknown\"?><a/>")
            == beresta::DomErrorCode::ENCODING_ERROR);
        TEST_ASSERT(beresta::encoding::findCharset("CP1251") != nullptr);
    }
}

namespace suites
{
    // Регистрация тестов кодировок
    void registerEncodingTests(test::TestRunner& runner)
    {
        runner.add("кодировки: windows-1251 и KOI8-R", testCyrillicSingleByte);
        runner.add("кодировки: UTF-16 с суррогатами", testUtf16);
        runner.add("кодировки: принудительная кодировка", testForcedEncoding);
        runner.add("кодировки: ошибки", testEncodingErrors);
    }
}
