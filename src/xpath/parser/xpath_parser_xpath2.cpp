#include "xpath/internal/xpath_parser.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    /*
    ** Одиночное выражение (ExprSingle XPath 2.0): в утверждениях XSD —
    ** if (…) then … else … или квантор some / every; иначе (и всегда в
    ** XPath 1.0) — бинарное выражение самого низкого приоритета. Квантор
    ** узнаётся по паре «some / every» + «$имя»: просто some — это путь.
    **
    ** A single expression (XPath 2.0 ExprSingle): in XSD assertions —
    ** if (…) then … else … or a some / every quantifier; otherwise (and always in
    ** XPath 1.0) — a binary expression of the lowest precedence. A quantifier
    ** is recognized by the pair "some / every" + "$name": a bare some is a path.
    */
    ExprPtr XPathParser::parseExprSingle()
    {
        const Token& token = peek();

        if (_dialect != XPathDialect::XSD_ASSERTION)
            return parseBinary(0);
        if (token.type == TokenType::FUNCTION_NAME && token.text == "if")
            return parseIf();
        bool quantifier = token.type == TokenType::NAME_TEST && (token.text == "some" || token.text == "every")
            && _tokens[_index + 1].type == TokenType::VARIABLE;
        if (quantifier)
            return parseQuantified();
        return parseBinary(0);
    }

    // if ( условие ) then выражение else выражение — ветвь else обязательна
    // if ( condition ) then expression else expression — the else branch is mandatory
    ExprPtr XPathParser::parseIf()
    {
        take();
        expect(TokenType::LEFT_PAREN, "'('");
        ExprPtr condition = parseExprSingle();
        expect(TokenType::RIGHT_PAREN, "')'");
        expectKeyword("then");
        ExprPtr thenBranch = parseExprSingle();
        expectKeyword("else");
        ExprPtr elseBranch = parseExprSingle();
        return std::make_unique<IfExpr>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
    }

    /*
    ** some | every $a in X (, $b in Y)* satisfies условие. Привязки
    ** собираются по порядку, а затем сворачиваются с конца во вложенные
    ** кванторы: every $a in X, $b in Y satisfies P = every $a in X satisfies
    ** (every $b in Y satisfies P). Цикл вместо рекурсии.
    **
    ** some | every $a in X (, $b in Y)* satisfies condition. The bindings
    ** are collected in order, and then folded from the end into nested
    ** quantifiers: every $a in X, $b in Y satisfies P = every $a in X satisfies
    ** (every $b in Y satisfies P). A loop instead of recursion.
    */
    ExprPtr XPathParser::parseQuantified()
    {
        bool every = take().text == "every";
        std::vector<std::pair<std::string, ExprPtr>> bindings;

        do
        {
            if (peek().type != TokenType::VARIABLE)
                fail(i18n::format(i18n::Message::XPATH_EXPECTED_FOUND, {"'$'", peek().text}));
            std::string variable = take().text;
            expectKeyword("in");
            bindings.emplace_back(std::move(variable), parseExprSingle());
        }
        while (accept(TokenType::COMMA));
        expectKeyword("satisfies");
        ExprPtr condition = parseExprSingle();
        for (std::size_t index = bindings.size(); index > 0; --index)
            condition = std::make_unique<QuantifiedExpr>(every, std::move(bindings[index - 1].first),
                std::move(bindings[index - 1].second), std::move(condition));
        return condition;
    }

    // Обязательное ключевое слово XPath 2.0 (then, else, in, satisfies)
    // A mandatory XPath 2.0 keyword (then, else, in, satisfies)
    void XPathParser::expectKeyword(const char* word)
    {
        if (peek().type != TokenType::OPERATOR_NAME || peek().text != word)
            fail(i18n::format(i18n::Message::XPATH_EXPECTED_FOUND, {std::string("'") + word + "'", peek().text}));
        take();
    }
}
