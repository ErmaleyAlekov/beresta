#include "xpath/internal/xpath_parser.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

namespace beresta::xpath::detail
{
    /*
    ** Проверка узла: имя / prefix:имя / * / prefix:* или тип узла:
    ** node(), text(), comment(), processing-instruction('цель'?).
    **
    ** A node test: name / prefix:name / * / prefix:* or a node type:
    ** node(), text(), comment(), processing-instruction('target'?).
    */
    void XPathParser::parseNodeTest(Step& step)
    {
        if (peek().type == TokenType::NODE_TYPE)
        {
            parseNodeTypeTest(step);
            return;
        }
        if (peek().type != TokenType::NAME_TEST)
            fail(i18n::format(i18n::Message::XPATH_EXPECTED_NODE_TEST));
        std::string name = take().text;
        std::size_t colon = name.find(':');
        step.test = TestKind::QUALIFIED_NAME;
        if (name == "*")
            step.test = TestKind::ANY_NAME;
        if (colon != std::string::npos)
            step.prefix = name.substr(0, colon);
        step.localName = name.substr(colon + 1);
        if (colon != std::string::npos && step.localName == "*")
            step.test = TestKind::NAMESPACE_ANY;
    }

    // Текущая лексема
    // The current token
    const Token& XPathParser::peek() const noexcept
    {
        return _tokens[_index];
    }

    // Забирает текущую лексему (END_OF_INPUT не пропускается)
    // Takes the current token (END_OF_INPUT is not skipped)
    Token XPathParser::take()
    {
        Token token = _tokens[_index];

        if (token.type != TokenType::END_OF_INPUT)
            ++_index;
        return token;
    }

    // Забирает лексему, если она нужного вида
    // Takes the token if it is of the required kind
    bool XPathParser::accept(TokenType type)
    {
        if (peek().type != type)
            return false;
        take();
        return true;
    }

    // Обязательная лексема
    // A mandatory token
    void XPathParser::expect(TokenType type, const char* description)
    {
        if (!accept(type))
            fail(i18n::format(i18n::Message::XPATH_EXPECTED_FOUND, {description, peek().text}));
    }
}
