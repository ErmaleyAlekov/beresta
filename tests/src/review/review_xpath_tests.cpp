#include "suites/test_suites.hpp"

#include "beresta.hpp"
#include "xpath/internal/xpath_assertion.hpp"
#include "xpath/internal/xpath_parser.hpp"

#include <string>

namespace
{
    using namespace beresta;
    using namespace beresta::xpath;
    using namespace beresta::xpath::detail;

    // Удвоенный ограничитель в утверждении обозначает ровно одну кавычку.
    // A doubled delimiter in an assertion denotes exactly one quote.
    void testDoubledQuotes()
    {
        Document document = XmlParser().parse("<a/>");
        for (const char* text : {R"('it''s' = "it's")", R"("a""b" = 'a"b')"})
        {
            auto assertion = ExpressionAccess::compileAssertion(text, {});
            TEST_ASSERT(ExpressionAccess::evaluateAssertion(assertion, document, {}, UntypedValueSource{}, nullptr));
            TEST_ASSERT_THROWS((void)XPathExpression{text}, DomException);
        }
    }

    // Вторая привязка видит первую, а satisfies — обе.
    // The second binding sees the first, and satisfies sees both.
    void testDependentBindings()
    {
        Document document = XmlParser().parse("<a/>");
        auto assertion = ExpressionAccess::compileAssertion("some $x in 7, $y in $x + 1 satisfies $x = 7 and $y = 8", {});
        TEST_ASSERT(ExpressionAccess::evaluateAssertion(assertion, document, {}, UntypedValueSource{}, nullptr));
    }

    // Кванторы в списке и в условии делят общий предел глубины.
    // Quantifiers in a list and in its condition share the same depth limit.
    void testBindingDepth()
    {
        std::string bindings = "some $x0 in 1";
        for (unsigned index = 1; index < 127; ++index)
            bindings += ", $x" + std::to_string(index) + " in 1";
        const std::string condition = " satisfies 1";
        ExpressionAccess::compileAssertion(bindings + condition, {});
        TEST_ASSERT_THROWS(ExpressionAccess::compileAssertion(bindings + ", $next in 1" + condition, {}), DomException);
        TEST_ASSERT_THROWS(ExpressionAccess::compileAssertion("some $outer in 1 satisfies " + bindings + condition, {}), DomException);
    }

    // Ручной поток без терминатора заканчивается после последней лексемы.
    // A manual stream without a terminator ends after its last token.
    void testTokenStreams()
    {
        constexpr auto dialect = XPathDialect::XSD_ASSERTION;
        XPathParser empty({}, dialect);
        XPathParser number({{TokenType::NUMBER, "7", 7.0}}, dialect);
        XPathParser name({{TokenType::NAME_TEST, "some", 0.0}}, dialect);
        Document document = XmlParser().parse("<some/>");
        XPathContext namespaces;
        EvalContext context{XPathItem{&document, nullptr, nullptr}, 1, 1, namespaces};
        TEST_ASSERT_THROWS(empty.parse(), DomException);
        TEST_ASSERT(number.parse()->evaluate(context).toNumber() == 7.0);
        auto result = name.parse()->evaluate(context);
        TEST_ASSERT(result.getNodes().size() == 1);
        TEST_ASSERT(result.getNodes().front().node == document.getRoot());
        TEST_ASSERT(XPathExpression("child :: node()").selectElements(document).size() == 1);
        TEST_ASSERT_THROWS((void)XPathExpression{"child : : node()"}, DomException);
    }
}

namespace suites
{
    // Регистрация регрессий разбора XPath.
    // Registers XPath parser regression tests.
    void registerReviewXPathTests(test::TestRunner& runner)
    {
        runner.add("Ревью Т-15: кавычки в литералах утверждений", testDoubledQuotes);
        runner.add("Ревью Т-15: зависимые привязки квантора", testDependentBindings);
        runner.add("Ревью Т-15: предел вложенности привязок", testBindingDepth);
        runner.add("Ревью Т-15: поток лексем без терминатора", testTokenStreams);
    }
}
