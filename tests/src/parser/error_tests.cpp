#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Позиция ошибки указывает на начало неверного закрывающего тега
    // The error position points to the start of the wrong end tag
    void testMismatchedTagPosition()
    {
        beresta::ParsePosition position = support::parseErrorPosition(
            "<root>\n  <item>\n  </itme>\n</root>");

        TEST_ASSERT(position.line == 3);
        TEST_ASSERT(position.column == 3);
    }

    // Столбец считается в символах, а не в байтах UTF-8
    // The column is counted in characters, not in UTF-8 bytes
    void testColumnCountsUtf8Characters()
    {
        beresta::ParsePosition position = support::parseErrorPosition("<я>ёж&bad;</я>");

        TEST_ASSERT(position.line == 1);
        TEST_ASSERT(position.column == 6);
    }

    // Структурные ошибки: незакрытый тег, лишний закрывающий, второй корень, пустота
    // Structural errors: an unclosed tag, an extra end tag, a second root, emptiness
    void testStructuralErrors()
    {
        beresta::XmlParser parser;

        TEST_ASSERT_THROWS(parser.parse("<a><b></a>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<a>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<a/></a>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<a/><b/>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("text<a/>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("   "), beresta::ParseException);
    }

    // Лексические ошибки: атрибуты, сущности, комментарии, объявления, декларация
    // Lexical errors: attributes, entities, comments, declarations, the XML declaration
    void testLexicalErrors()
    {
        beresta::XmlParser parser;

        TEST_ASSERT_THROWS(parser.parse("<a x=\"1\" x=\"2\"/>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<a x=\"1\"y=\"2\"/>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<a x=1/>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<a>&nope;</a>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<a>&#xD800;</a>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<a><!-- a -- b --></a>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<!FOO a><a/>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<a/><?xml version=\"1.0\"?>"), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<![CDATA[x]]><a/>"), beresta::ParseException);
    }
}

namespace suites
{
    // Регистрация тестов сообщений об ошибках
    // Registers the error message tests
    void registerErrorTests(test::TestRunner& runner)
    {
        runner.add("ошибки: позиция несовпадающего тега", testMismatchedTagPosition);
        runner.add("ошибки: столбец в символах UTF-8", testColumnCountsUtf8Characters);
        runner.add("ошибки: структурные", testStructuralErrors);
        runner.add("ошибки: лексические", testLexicalErrors);
    }
}
