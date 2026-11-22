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
        position = support::parseErrorPosition("<root>\r\n  <item>\r\n  </itme></root>");
        TEST_ASSERT(position.line == 3);
        TEST_ASSERT(position.column == 3);
    }

    // Столбец считается в символах, а ошибки в сущности отмечаются у ссылки.
    // The column counts characters, and entity errors point to the reference.
    void testErrorPositions()
    {
        beresta::ParsePosition position = support::parseErrorPosition("<я>ёж&bad;</я>");

        TEST_ASSERT(position.line == 1);
        TEST_ASSERT(position.column == 6);
        // Ошибка раскрытия значения относится к открывающему тегу.
        // A value expansion error is anchored at its opening tag.
        const auto value = support::parseErrorPosition("<root>\r\n  <a x=\"1\r\n    &bad;\"/></root>");
        const auto attribute = support::parseErrorPosition("<a x=\"1\n  2\"\n  x=\"3\"/>");
        const auto entity = support::parseErrorPosition(
            "<!DOCTYPE r [<!ENTITY e \"<x></y>\">]>\n<r>\n  &e;\n</r>");

        TEST_ASSERT(value.line == 2);
        TEST_ASSERT(value.column == 3);
        TEST_ASSERT(attribute.line == 3);
        TEST_ASSERT(attribute.column == 3);
        TEST_ASSERT(entity.line == 3);
        TEST_ASSERT(entity.column == 3);
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
        runner.add("ошибки: UTF-8, атрибут и ссылка на сущность", testErrorPositions);
        runner.add("ошибки: структурные", testStructuralErrors);
        runner.add("ошибки: лексические", testLexicalErrors);
    }
}
