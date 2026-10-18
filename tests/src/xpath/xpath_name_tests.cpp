#include "suites/test_suites.hpp"

#include "beresta.hpp"

/*
** Имена в выражениях XPath проверяются по тем же таблицам XML 1.0
** 5-й редакции, что и в парсере (раньше любой байт >= 0x80 был буквой).
**
** Names in XPath expressions are checked against the same XML 1.0
** 5th edition tables as in the parser (previously any byte >= 0x80 was a letter).
*/
namespace
{
    // Кириллица и U+00B7 внутри имени допустимы, U+00D7 (×) — нет ни в начале, ни внутри
    // Cyrillic and U+00B7 inside a name are allowed, U+00D7 (×) is not, neither at the start nor inside
    void testUnicodeNames()
    {
        beresta::Document document = beresta::XmlParser().parse(
            "<корень><узел a\xC2\xB7" "b=\"1\">x</узел><узел/></корень>");

        TEST_ASSERT(beresta::xpath::selectElements(document, "/корень/узел").size() == 2);
        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, "//узел/@a\xC2\xB7" "b"), "1");
        TEST_ASSERT_THROWS(beresta::xpath::XPathExpression("/\xC3\x97"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xpath::XPathExpression("/a\xC3\x97"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xpath::XPathExpression("/\xC2\xB7"), beresta::DomException);
    }
}

namespace suites
{
    // Регистрация тестов имён XPath
    // Registers the XPath name tests
    void registerXPathNameTests(test::TestRunner& runner)
    {
        runner.add("XPath: имена по таблицам 5-й редакции", testUnicodeNames);
    }
}
