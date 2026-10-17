#ifndef XPATH_PARSER_HPP
#define XPATH_PARSER_HPP

#include "xpath/internal/xpath_ast.hpp"
#include "xpath/internal/xpath_lexer.hpp"

#include <cstddef>
#include <vector>

namespace beresta::xpath::detail
{
    /*
    ** XPathParser — рекурсивный спуск по грамматике XPath 1.0 (раздел 3).
    ** Шесть уровней приоритета бинарных операций разбираются одной
    ** функцией по таблице уровней (DRY): or < and < = != < сравнения <
    ** + - < * div mod. Результат — дерево Expr (паттерн Interpreter).
    */
    class XPathParser
    {
    public:
        explicit XPathParser(std::vector<Token> tokens);

        // Всё выражение целиком; лишние лексемы в конце — ошибка
        ExprPtr parse();

    private:
        // ---- Выражения (xpath_parser.cpp) ----
        ExprPtr parseBinary(std::size_t level);
        ExprPtr parseUnary();
        ExprPtr parseUnion();
        ExprPtr parsePath();

        // ---- Первичные выражения (xpath_parser_primary.cpp) ----
        ExprPtr parseFilter();
        ExprPtr parsePrimary();
        ExprPtr parseFunctionCall();
        void parsePredicates(std::vector<ExprPtr>& predicates);
        bool startsLocationPath() const noexcept;

        // ---- Пути (xpath_parser_paths.cpp) ----
        ExprPtr parseLocationPath();
        void parseRelativePath(std::vector<Step>& steps);
        Step parseStep();
        void parseNodeTest(Step& step);
        void parseNodeTypeTest(Step& step);
        Step descendantStep() const;

        // ---- Лексемы (xpath_parser_tokens.cpp) ----
        const Token& peek() const noexcept;
        Token take();
        bool accept(TokenType type);
        void expect(TokenType type, const char* description);
        [[noreturn]] void fail(const std::string& message) const;

        std::vector<Token> _tokens;
        std::size_t _index;
    };
}

#endif
