#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Значение выражения строкой относительно документа
    // The value of an expression as a string relative to the document
    std::string eval(const char* xml, const char* expression)
    {
        beresta::Document document = beresta::XmlParser().parse(xml);

        return beresta::xpath::evaluateString(document, expression);
    }

    // Строковые функции работают по символам Unicode, а не по байтам
    // String functions work by Unicode characters, not by bytes
    void testStringFunctions()
    {
        const char* xml = "<r><s>  Привет,   мир  </s></r>";

        TEST_ASSERT_EQ(eval(xml, "normalize-space(//s)"), "Привет, мир");
        TEST_ASSERT_EQ(eval(xml, "string-length(normalize-space(//s))"), "11");
        TEST_ASSERT_EQ(eval(xml, "substring(normalize-space(//s), 2, 3)"), "рив");
        TEST_ASSERT_EQ(eval(xml, "translate('абв', 'аб', 'AB')"), "ABв");
        TEST_ASSERT_EQ(eval(xml, "concat('a', 1, true())"), "a1true");
        TEST_ASSERT_EQ(eval(xml, "substring-before('2026-10-17', '-')"), "2026");
        TEST_ASSERT_EQ(eval(xml, "substring-after('2026-10-17', '-')"), "10-17");
        TEST_ASSERT_EQ(eval(xml, "contains(//s, 'мир') and starts-with('abc', 'ab')"), "true");
    }

    // Числа: арифметика, форматирование (кратчайшее точное, как требует XPath 4.2), NaN, Infinity
    // Numbers: arithmetic, formatting (the shortest exact one, as XPath 4.2 requires), NaN, Infinity
    void testNumberFunctions()
    {
        const char* xml = "<r><n>10</n><n>2.5</n><n>x</n></r>";

        TEST_ASSERT_EQ(eval(xml, "sum(//n[number(.) = number(.)])"), "12.5");
        TEST_ASSERT_EQ(eval(xml, "number(//n[3])"), "NaN");
        TEST_ASSERT_EQ(eval(xml, "7 mod 3 + 7 div 2"), "4.5");
        TEST_ASSERT_EQ(eval(xml, "1 div 0"), "Infinity");
        TEST_ASSERT_EQ(eval(xml, "round(2.5) + floor(-1.5) + ceiling(1.2)"), "3");
        TEST_ASSERT_EQ(eval(xml, "0.1 + 0.2"), "0.30000000000000004");
        TEST_ASSERT_EQ(eval(xml, "0.5 + 0.25"), "0.75");
        TEST_ASSERT_EQ(eval(xml, "1000000 * 1000000 * 1000000"), "1000000000000000000");
    }

    // Сравнения множеств, логика, имена, lang() и переменные
    // Set comparisons, logic, names, lang() and variables
    void testLogicAndNames()
    {
        const char* xml = "<r xml:lang=\"ru-RU\"><a n=\"1\"/><a n=\"2\"/><p:b xmlns:p=\"urn:p\"/></r>";
        beresta::Document document = beresta::XmlParser().parse(xml);
        beresta::xpath::XPathContext context;
        context.variables.emplace("limit", beresta::xpath::XPathValue::fromNumber(1));

        TEST_ASSERT_EQ(eval(xml, "//a/@n = 2"), "true");
        TEST_ASSERT_EQ(eval(xml, "//a/@n != 1"), "true");
        TEST_ASSERT_EQ(eval(xml, "not(//a/@n > 5)"), "true");
        TEST_ASSERT_EQ(eval(xml, "local-name(/r/*[3])"), "b");
        TEST_ASSERT_EQ(eval(xml, "namespace-uri(/r/*[3])"), "urn:p");
        TEST_ASSERT_EQ(eval(xml, "boolean(//a[lang('ru')])"), "true");
        TEST_ASSERT_EQ(beresta::xpath::XPathExpression("count(//a[@n > $limit])")
            .evaluate(document, context).toString(), "1");
        TEST_ASSERT_THROWS(beresta::xpath::XPathExpression("//a["), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xpath::XPathExpression("unknown()").evaluate(document),
            beresta::DomException);
    }
}

namespace suites
{
    // Регистрация тестов функций XPath
    // Registers the XPath function tests
    void registerXPathFunctionTests(test::TestRunner& runner)
    {
        runner.add("XPath: строковые функции", testStringFunctions);
        runner.add("XPath: числа", testNumberFunctions);
        runner.add("XPath: логика, имена и переменные", testLogicAndNames);
    }
}
