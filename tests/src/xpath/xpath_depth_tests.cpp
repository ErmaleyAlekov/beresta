#include "suites/test_suites.hpp"

#include "beresta.hpp"

#include <string>

/*
** Разбор XPath — рекурсивный спуск, поэтому глубина вложенности
** выражения ограничена: сверх предела — ошибка XPATH_ERROR, а не
** переполнение стека (раньше ~1000 скобок роняли процесс). Длинные
** цепочки операторов не вложены, а дают глубокое дерево: они
** вычисляются и освобождаются циклом.
**
** XPath parsing is recursive descent, so the nesting depth of an
** expression is limited: beyond the limit it is an XPATH_ERROR error rather
** than a stack overflow (before, ~1000 parentheses crashed the process). Long
** operator chains are not nested but give a deep tree: they
** are evaluated and freed by a loop.
*/
namespace
{
    // Строка из count повторов фрагмента part
    // A string of count repetitions of the fragment part
    std::string repeat(const std::string& part, std::size_t count)
    {
        std::string result;

        for (std::size_t index = 0; index < count; ++index)
            result += part;
        return result;
    }

    // 100 уровней скобок, аргументов функций и предикатов компилируются и вычисляются
    // 100 levels of parentheses, function arguments and predicates compile and evaluate
    void testReasonableDepth()
    {
        beresta::Document document = beresta::XmlParser().parse("<a><a/></a>");

        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, repeat("(", 100) + "7" + repeat(")", 100)), "7");
        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document,
            repeat("not(", 100) + "true()" + repeat(")", 100)), "true");
        beresta::xpath::XPathExpression predicates(repeat("a[", 100) + "1" + repeat("]", 100));
        TEST_ASSERT(predicates.selectElements(document).empty());
    }

    // 100 000 уровней — ошибка XPath, а не падение процесса
    // 100,000 levels are an XPath error rather than a process crash
    void testExtremeDepth()
    {
        const std::size_t depth = 100000;

        TEST_ASSERT_THROWS(beresta::xpath::XPathExpression(repeat("(", depth) + "1" + repeat(")", depth)),
            beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xpath::XPathExpression(repeat("not(", depth) + "true()" + repeat(")", depth)),
            beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xpath::XPathExpression(repeat("a[", depth) + "1" + repeat("]", depth)),
            beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xpath::XPathExpression(repeat("concat('', ", depth) + "'x'" + repeat(")", depth)),
            beresta::DomException);
    }

    /*
    ** Цепочки из 100 000 операторов вычисляются и освобождаются без
    ** переполнения стека: арифметика, or с коротким замыканием в конце,
    ** объединение множеств; 100 000 унарных минусов разбираются циклом.
    **
    ** Chains of 100,000 operators are evaluated and freed without
    ** a stack overflow: arithmetic, or with short-circuiting at the end,
    ** set union; 100,000 unary minuses are parsed by a loop.
    */
    void testLongChains()
    {
        const std::size_t length = 100000;
        beresta::Document document = beresta::XmlParser().parse("<a><b/></a>");

        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, "1" + repeat("+1", length)), "100001");
        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, "2" + repeat(" - 1", length) + " * 0"), "-99997");
        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, "false()" + repeat(" or false()", length)
            + " or true()"), "true");
        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, "count(/a/b" + repeat(" | /a/b", length) + ")"), "1");
        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, repeat("-", length) + "5"), "5");
        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, repeat("-", length - 1) + "5"), "-5");
    }
}

namespace suites
{
    // Регистрация тестов глубины выражений XPath
    // Registers the XPath expression depth tests
    void registerXPathDepthTests(test::TestRunner& runner)
    {
        runner.add("XPath: 100 уровней вложенности разбираются", testReasonableDepth);
        runner.add("XPath: 100 000 уровней — ошибка, а не падение", testExtremeDepth);
        runner.add("XPath: цепочки из 100 000 операторов", testLongChains);
    }
}
