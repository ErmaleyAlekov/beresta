#include "xpath/internal/xpath_parser.hpp"
#include "exception/dom_exception.hpp"

namespace beresta::xpath::detail
{
    // Ноль или больше предикатов [выражение]
    void XPathParser::parsePredicates(std::vector<ExprPtr>& predicates)
    {
        while (accept(TokenType::LEFT_BRACKET))
        {
            predicates.push_back(parseBinary(0));
            expect(TokenType::RIGHT_BRACKET, "']'");
        }
    }

    // Сокращение "//" — это шаг descendant-or-self::node()
    Step XPathParser::descendantStep() const
    {
        Step step;

        step.axis = Axis::DESCENDANT_OR_SELF;
        step.test = TestKind::ANY_NODE;
        return step;
    }

    // node() | text() | comment() | processing-instruction('цель'?)
    void XPathParser::parseNodeTypeTest(Step& step)
    {
        std::string type = take().text;

        expect(TokenType::LEFT_PAREN, "'('");
        step.test = TestKind::ANY_NODE;
        if (type == "text")
            step.test = TestKind::TEXT_NODE;
        else if (type == "comment")
            step.test = TestKind::COMMENT_NODE;
        else if (type == "processing-instruction")
            step.test = TestKind::PI_NODE;
        if (step.test == TestKind::PI_NODE && peek().type == TokenType::LITERAL)
            step.target = take().text;
        expect(TokenType::RIGHT_PAREN, "')'");
    }

    // Синтаксическая ошибка с номером лексемы
    void XPathParser::fail(const std::string& message) const
    {
        throw DomException(DomErrorCode::XPATH_ERROR,
            "XPath, лексема " + std::to_string(_index + 1) + ": " + message);
    }
}
