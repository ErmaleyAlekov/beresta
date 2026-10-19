#ifndef XPATH_PARSER_HPP
#define XPATH_PARSER_HPP

#include "xpath/internal/xpath_ast.hpp"
#include "xpath/internal/xpath_lexer.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace beresta::xpath::detail
{
    /*
    ** XPathParser — рекурсивный спуск по грамматике XPath 1.0 (раздел 3).
    ** Шесть уровней приоритета бинарных операций разбираются одной
    ** функцией по таблице уровней (DRY): or < and < = != < сравнения <
    ** + - < * div mod. Результат — дерево Expr (паттерн Interpreter).
    ** В диалекте утверждений XSD на месте «одиночного выражения» (весь
    ** текст, скобки, аргумент, предикат, ветви) допустимы if и some / every,
    ** а имена функций проверяются сразу: неподдерживаемая конструкция
    ** XPath 2.0 — ошибка компиляции, то есть ошибка загрузки схемы.
    **
    ** XPathParser — recursive descent over the XPath 1.0 grammar (section 3).
    ** The six precedence levels of binary operations are parsed by one
    ** function using a table of levels (DRY): or < and < = != < comparisons <
    ** + - < * div mod. The result is an Expr tree (the Interpreter pattern).
    ** In the XSD assertion dialect if and some / every are allowed in place of a
    ** "single expression" (the whole text, brackets, an argument, a predicate, branches),
    ** and function names are checked at once: an unsupported XPath 2.0
    ** construct is a compilation error, that is a schema loading error.
    */
    class XPathParser
    {
    public:
        // namespaces — префиксы для конструкторов xs:* (только для диалекта утверждений)
        // namespaces — the prefixes for xs:* constructors (only for the assertion dialect)
        explicit XPathParser(std::vector<Token> tokens, XPathDialect dialect = XPathDialect::XPATH_1_0,
            const XPathContext* namespaces = nullptr);

        // Всё выражение целиком; лишние лексемы в конце — ошибка
        // The whole expression; extra tokens at the end are an error
        ExprPtr parse();

    private:
        // ---- Выражения (xpath_parser.cpp) ----
        // ---- Expressions (xpath_parser.cpp) ----
        ExprPtr parseBinary(std::size_t level);
        ExprPtr parseUnary();
        ExprPtr parseUnion();
        ExprPtr parsePath();

        // ---- Конструкции XPath 2.0 (xpath_parser_xpath2.cpp, xpath_parser_functions.cpp) ----
        // ---- XPath 2.0 constructs (xpath_parser_xpath2.cpp, xpath_parser_functions.cpp) ----
        ExprPtr parseExprSingle();
        ExprPtr parseIf();
        ExprPtr parseQuantified();
        void expectKeyword(const char* word);
        ExprPtr assertionCall(std::string name, std::vector<ExprPtr> arguments);

        // ---- Первичные выражения (xpath_parser_primary.cpp) ----
        // ---- Primary expressions (xpath_parser_primary.cpp) ----
        ExprPtr parseFilter();
        ExprPtr parsePrimary();
        ExprPtr parseFunctionCall();
        void parsePredicates(std::vector<ExprPtr>& predicates);
        bool startsLocationPath() const noexcept;

        // ---- Пути (xpath_parser_paths.cpp) ----
        // ---- Paths (xpath_parser_paths.cpp) ----
        ExprPtr parseLocationPath();
        void parseRelativePath(std::vector<Step>& steps);
        Step parseStep();
        void parseNodeTest(Step& step);
        void parseNodeTypeTest(Step& step);
        Step descendantStep() const;

        // ---- Лексемы (xpath_parser_tokens.cpp) ----
        // ---- Tokens (xpath_parser_tokens.cpp) ----
        const Token& peek() const noexcept;
        Token take();
        bool accept(TokenType type);
        void expect(TokenType type, const char* description);
        [[noreturn]] void fail(const std::string& message) const;

        std::vector<Token> _tokens;
        std::size_t _index;
        XPathDialect _dialect;
        const XPathContext* _namespaces;
    };
}

#endif
