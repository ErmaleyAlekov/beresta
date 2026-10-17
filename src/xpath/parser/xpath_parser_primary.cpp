#include "xpath/internal/xpath_parser.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Объединение множеств узлов через '|'
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
    ExprPtr XPathParser::parseFilter()
    {
        ExprPtr primary = parsePrimary();
        std::vector<ExprPtr> predicates;

        parsePredicates(predicates);
        if (predicates.empty())
            return primary;
        return std::make_unique<FilterExpr>(std::move(primary), std::move(predicates));
    }

    // $переменная | (выражение) | 'литерал' | число | функция(...)
    ExprPtr XPathParser::parsePrimary()
    {
        TokenType type = peek().type;

        if (type == TokenType::VARIABLE)
            return std::make_unique<VariableExpr>(take().text);
        if (type == TokenType::LITERAL)
            return std::make_unique<ConstantExpr>(XPathValue::fromString(take().text));
        if (type == TokenType::NUMBER)
            return std::make_unique<ConstantExpr>(XPathValue::fromNumber(take().number));
        if (type == TokenType::FUNCTION_NAME)
            return parseFunctionCall();
        expect(TokenType::LEFT_PAREN, "'('");
        ExprPtr inner = parseBinary(0);
        expect(TokenType::RIGHT_PAREN, "')'");
        return inner;
    }

    // имя(аргумент, аргумент, ...)
    ExprPtr XPathParser::parseFunctionCall()
    {
        std::string name = take().text;
        std::vector<ExprPtr> arguments;

        expect(TokenType::LEFT_PAREN, "'('");
        if (!accept(TokenType::RIGHT_PAREN))
        {
            arguments.push_back(parseBinary(0));
            while (accept(TokenType::COMMA))
                arguments.push_back(parseBinary(0));
            expect(TokenType::RIGHT_PAREN, "')'");
        }
        return std::make_unique<FunctionCallExpr>(std::move(name), std::move(arguments));
    }
}
