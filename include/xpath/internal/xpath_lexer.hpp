#ifndef XPATH_LEXER_HPP
#define XPATH_LEXER_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::xpath::detail
{
    // Вид лексемы XPath 1.0 (раздел 3.7 спецификации)
    // The kind of an XPath 1.0 token (section 3.7 of the specification)
    enum class TokenType
    {
        NUMBER, LITERAL, NAME_TEST, NODE_TYPE, FUNCTION_NAME, AXIS_NAME, VARIABLE,
        OPERATOR_NAME, MULTIPLY, SLASH, DOUBLE_SLASH, PIPE, PLUS, MINUS_SIGN,
        EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
        LEFT_PAREN, RIGHT_PAREN, LEFT_BRACKET, RIGHT_BRACKET, DOT, DOUBLE_DOT,
        AT_SIGN, COMMA, DOUBLE_COLON, END_OF_INPUT
    };

    // Лексема: вид, текст (имя, литерал) и значение числа
    // A token: the kind, the text (a name, a literal) and the number value
    struct Token
    {
        TokenType type;
        std::string text;
        double number;
    };

    /*
    ** XPathLexer — разбиение выражения на лексемы с учётом правил
    ** однозначности: '*' и имена вроде "div" трактуются как операторы
    ** только после операнда; имя перед '(' — функция или тип узла;
    ** имя перед "::" — ось.
    **
    ** XPathLexer — splits an expression into tokens following the
    ** disambiguation rules: '*' and names like "div" are treated as operators
    ** only after an operand; a name before '(' is a function or a node type;
    ** a name before "::" is an axis.
    */
    class XPathLexer
    {
    public:
        explicit XPathLexer(std::string_view source) noexcept;

        // Все лексемы; последняя — END_OF_INPUT. Ошибка — DomException(XPATH_ERROR)
        // All tokens; the last one is END_OF_INPUT. An error is DomException(XPATH_ERROR)
        std::vector<Token> tokenize();

    private:
        Token readToken();
        Token readNumber();
        Token readLiteral();
        Token readSymbol();
        Token readNameLike();

        // Префикс, ось, функция или тип узла после прочитанного NCName
        // A prefix, an axis, a function or a node type after the NCName that was read
        Token classifyName(std::string name);

        // Можно ли в этой позиции ожидать оператор (предыдущая лексема — операнд)
        // Whether an operator can be expected at this position (the previous token is an operand)
        bool operatorExpected() const noexcept;

        // NCName (имя без двоеточия)
        // NCName (a name without a colon)
        std::string readNcName();

        // Первый непробельный символ начиная с позиции from
        // The first non-whitespace character starting from position from
        char lookAhead(std::size_t from) const noexcept;

        // Строит лексему и проверяет её; при ошибке — XPATH_ERROR с позицией
        // Builds a token and checks it; on error — XPATH_ERROR with the position
        Token makeToken(TokenType type, std::string text) const;

        [[noreturn]] void fail(const std::string& message) const;

        std::string_view _source;
        std::size_t _position;
        std::vector<Token> _tokens;
    };
}

#endif
