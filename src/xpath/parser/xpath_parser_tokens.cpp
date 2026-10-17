#include "xpath/internal/xpath_parser.hpp"
#include "exception/dom_exception.hpp"

namespace beresta::xpath::detail
{
    /*
    ** Проверка узла: имя / prefix:имя / * / prefix:* или тип узла:
    ** node(), text(), comment(), processing-instruction('цель'?).
    */
    void XPathParser::parseNodeTest(Step& step)
    {
        if (peek().type == TokenType::NODE_TYPE)
        {
            parseNodeTypeTest(step);
            return;
        }
        if (peek().type != TokenType::NAME_TEST)
            fail("ожидалась проверка узла (имя, * или node())");
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
    const Token& XPathParser::peek() const noexcept
    {
        return _tokens[_index];
    }

    // Забирает текущую лексему (END_OF_INPUT не пропускается)
    Token XPathParser::take()
    {
        Token token = _tokens[_index];

        if (token.type != TokenType::END_OF_INPUT)
            ++_index;
        return token;
    }

    // Забирает лексему, если она нужного вида
    bool XPathParser::accept(TokenType type)
    {
        if (peek().type != type)
            return false;
        take();
        return true;
    }

    // Обязательная лексема
    void XPathParser::expect(TokenType type, const char* description)
    {
        if (!accept(type))
            fail(std::string("ожидалось ") + description + ", найдено \"" + peek().text + "\"");
    }
}
