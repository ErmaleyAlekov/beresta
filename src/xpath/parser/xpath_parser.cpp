#include "xpath/internal/xpath_parser.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Количество уровней приоритета бинарных операций
    // The number of precedence levels of binary operations
    static constexpr std::size_t LEVEL_COUNT = 6;

    // Строка таблицы операторов: уровень приоритета, лексема и операция
    // A row of the operator table: the precedence level, the token and the operation
    struct OperatorEntry
    {
        std::size_t level;
        TokenType type;
        const char* text;
        BinaryOperator op;
    };

    /*
    ** Уровни: 0 — or, 1 — and, 2 — = != eq ne, 3 — < <= > >= lt le gt ge,
    ** 4 — + -, 5 — * div mod. Для OPERATOR_NAME дополнительно сравнивается
    ** текст лексемы; eq … ge лексер выдаёт только в диалекте утверждений.
    **
    ** Levels: 0 — or, 1 — and, 2 — = != eq ne, 3 — < <= > >= lt le gt ge,
    ** 4 — + -, 5 — * div mod. For OPERATOR_NAME the token text is compared
    ** too; the lexer produces eq … ge only in the assertion dialect.
    */
    static const OperatorEntry OPERATORS[] = {
        {0, TokenType::OPERATOR_NAME, "or", BinaryOperator::OR_OP},
        {1, TokenType::OPERATOR_NAME, "and", BinaryOperator::AND_OP},
        {2, TokenType::EQUAL, "", BinaryOperator::EQUAL_OP},
        {2, TokenType::NOT_EQUAL, "", BinaryOperator::NOT_EQUAL_OP},
        {2, TokenType::OPERATOR_NAME, "eq", BinaryOperator::VALUE_EQUAL_OP},
        {2, TokenType::OPERATOR_NAME, "ne", BinaryOperator::VALUE_NOT_EQUAL_OP},
        {3, TokenType::LESS, "", BinaryOperator::LESS_OP},
        {3, TokenType::LESS_EQUAL, "", BinaryOperator::LESS_EQUAL_OP},
        {3, TokenType::GREATER, "", BinaryOperator::GREATER_OP},
        {3, TokenType::GREATER_EQUAL, "", BinaryOperator::GREATER_EQUAL_OP},
        {3, TokenType::OPERATOR_NAME, "lt", BinaryOperator::VALUE_LESS_OP},
        {3, TokenType::OPERATOR_NAME, "le", BinaryOperator::VALUE_LESS_EQUAL_OP},
        {3, TokenType::OPERATOR_NAME, "gt", BinaryOperator::VALUE_GREATER_OP},
        {3, TokenType::OPERATOR_NAME, "ge", BinaryOperator::VALUE_GREATER_EQUAL_OP},
        {4, TokenType::PLUS, "", BinaryOperator::PLUS_OP},
        {4, TokenType::MINUS_SIGN, "", BinaryOperator::MINUS_OP},
        {5, TokenType::MULTIPLY, "", BinaryOperator::MULTIPLY_OP},
        {5, TokenType::OPERATOR_NAME, "div", BinaryOperator::DIVIDE_OP},
        {5, TokenType::OPERATOR_NAME, "mod", BinaryOperator::MODULO_OP}
    };

    // Распознаёт оператор уровня level в лексеме token
    // Recognizes an operator of level level in the token token
    static bool matchOperator(std::size_t level, const Token& token, BinaryOperator& op) noexcept
    {
        for (const OperatorEntry& entry : OPERATORS)
        {
            bool textOk = entry.type != TokenType::OPERATOR_NAME || token.text == entry.text;
            if (entry.level == level && entry.type == token.type && textOk)
            {
                op = entry.op;
                return true;
            }
        }
        return false;
    }

    // Парсер работает по готовому списку лексем в заданном диалекте
    // The parser works over a ready list of tokens in the given dialect
    XPathParser::XPathParser(std::vector<Token> tokens, XPathDialect dialect, const XPathContext* namespaces)
        : _tokens(std::move(tokens)), _index(0), _dialect(dialect), _namespaces(namespaces)
    {
    }

    // Всё выражение — одиночное выражение (в XPath 1.0 — бинарное самого низкого приоритета)
    // The whole expression is a single expression (in XPath 1.0 — a binary one of the lowest precedence)
    ExprPtr XPathParser::parse()
    {
        ExprPtr expression = parseExprSingle();

        if (peek().type != TokenType::END_OF_INPUT)
            fail(i18n::format(i18n::Message::XPATH_EXTRA_TOKEN, {peek().text}));
        return expression;
    }

    // Левоассоциативная цепочка операторов одного уровня
    // A left-associative chain of operators of one level
    ExprPtr XPathParser::parseBinary(std::size_t level)
    {
        if (level == LEVEL_COUNT)
            return parseUnary();
        ExprPtr left = parseBinary(level + 1);
        BinaryOperator op = BinaryOperator::OR_OP;
        while (matchOperator(level, peek(), op))
        {
            take();
            ExprPtr right = parseBinary(level + 1);
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }
        return left;
    }

    // Унарные минусы: чётное число взаимно уничтожается
    // Unary minuses: an even number cancels out
    ExprPtr XPathParser::parseUnary()
    {
        std::size_t count = 0;

        while (accept(TokenType::MINUS_SIGN))
            ++count;
        ExprPtr operand = parseUnion();
        if (count % 2 == 1)
            return std::make_unique<NegateExpr>(std::move(operand));
        return operand;
    }
}
