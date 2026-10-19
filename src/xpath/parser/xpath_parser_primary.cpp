#include "xpath/internal/xpath_parser.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Объединение множеств узлов через '|'
    // The union of node sets through '|'
    ExprPtr XPathParser::parseUnion()
    {
        ExprPtr left = parsePath();

        while (accept(TokenType::PIPE))
            left = std::make_unique<BinaryExpr>(BinaryOperator::UNION_OP, std::move(left), parsePath());
        return left;
    }

    /*
    ** Выражение пути: путь от корня/текущего узла или первичное выражение
    ** (переменная, скобки, литерал, функция), за которым может идти путь.
    **
    ** A path expression: a path from the root/the current node or a primary expression
    ** (a variable, brackets, a literal, a function) that may be followed by a path.
    */
    ExprPtr XPathParser::parsePath()
    {
        if (startsLocationPath())
            return parseLocationPath();
        ExprPtr filter = parseFilter();
        if (peek().type != TokenType::SLASH && peek().type != TokenType::DOUBLE_SLASH)
            return filter;
        std::vector<Step> steps;
        if (take().type == TokenType::DOUBLE_SLASH)
            steps.push_back(descendantStep());
        parseRelativePath(steps);
        return std::make_unique<PathExpr>(std::move(filter), false, std::move(steps));
    }

    // Первичное выражение с предикатами
    // A primary expression with predicates
    ExprPtr XPathParser::parseFilter()
    {
        ExprPtr primary = parsePrimary();
        std::vector<ExprPtr> predicates;

        parsePredicates(predicates);
        if (predicates.empty())
            return primary;
        return std::make_unique<FilterExpr>(std::move(primary), std::move(predicates));
    }

    /*
    ** $переменная | (выражение) | 'литерал' | число | функция(...). Число в
    ** утверждениях XSD — точный xs:decimal / xs:integer (как в XPath 2.0),
    ** в XPath 1.0 — double.
    **
    ** $variable | (expression) | 'literal' | number | function(...). A number in
    ** XSD assertions is an exact xs:decimal / xs:integer (as in XPath 2.0),
    ** in XPath 1.0 — a double.
    */
    ExprPtr XPathParser::parsePrimary()
    {
        TokenType type = peek().type;
        utils::Decimal exact;

        if (type == TokenType::VARIABLE)
            return std::make_unique<VariableExpr>(take().text);
        if (type == TokenType::LITERAL)
            return std::make_unique<ConstantExpr>(XPathValue::fromString(take().text));
        if (type == TokenType::NUMBER && _dialect == XPathDialect::XSD_ASSERTION
            && utils::readDecimal(peek().text, exact))
        {
            take();
            return std::make_unique<ConstantExpr>(XPathValue::fromDecimal(exact));
        }
        if (type == TokenType::NUMBER)
            return std::make_unique<ConstantExpr>(XPathValue::fromNumber(take().number));
        if (type == TokenType::FUNCTION_NAME)
            return parseFunctionCall();
        expect(TokenType::LEFT_PAREN, "'('");
        ExprPtr inner = parseExprSingle();
        expect(TokenType::RIGHT_PAREN, "')'");
        return inner;
    }

    // имя(аргумент, аргумент, ...); в утверждениях имя проверяется сразу
    // name(argument, argument, ...); in assertions the name is checked at once
    ExprPtr XPathParser::parseFunctionCall()
    {
        std::string name = take().text;
        std::vector<ExprPtr> arguments;

        expect(TokenType::LEFT_PAREN, "'('");
        if (!accept(TokenType::RIGHT_PAREN))
        {
            arguments.push_back(parseExprSingle());
            while (accept(TokenType::COMMA))
                arguments.push_back(parseExprSingle());
            expect(TokenType::RIGHT_PAREN, "')'");
        }
        if (_dialect == XPathDialect::XSD_ASSERTION)
            return assertionCall(std::move(name), std::move(arguments));
        return std::make_unique<FunctionCallExpr>(std::move(name), std::move(arguments));
    }
}
