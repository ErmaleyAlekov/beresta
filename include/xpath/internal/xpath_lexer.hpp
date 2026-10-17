#ifndef XPATH_LEXER_HPP
#define XPATH_LEXER_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::xpath::detail
{
    // Вид лексемы XPath 1.0 (раздел 3.7 спецификации)
    enum class TokenType
    {
        NUMBER, LITERAL, NAME_TEST, NODE_TYPE, FUNCTION_NAME, AXIS_NAME, VARIABLE,
        OPERATOR_NAME, MULTIPLY, SLASH, DOUBLE_SLASH, PIPE, PLUS, MINUS_SIGN,
        EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
        LEFT_PAREN, RIGHT_PAREN, LEFT_BRACKET, RIGHT_BRACKET, DOT, DOUBLE_DOT,
        AT_SIGN, COMMA, DOUBLE_COLON, END_OF_INPUT
    };

    // Лексема: вид, текст (имя, литерал) и значение числа
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
    */
    class XPathLexer
    {
    public:
        explicit XPathLexer(std::string_view source) noexcept;

        // Все лексемы; последняя — END_OF_INPUT. Ошибка — DomException(XPATH_ERROR)
        std::vector<Token> tokenize();

    private:
        Token readToken();
        Token readNumber();
        Token readLiteral();
        Token readSymbol();
        Token readNameLike();

        // Префикс, ось, функция или тип узла после прочитанного NCName
        Token classifyName(std::string name);

        // Можно ли в этой позиции ожидать оператор (предыдущая лексема — операнд)
        bool operatorExpected() const noexcept;

        // NCName (имя без двоеточия)
        std::string readNcName();

        // Первый непробельный символ начиная с позиции from
        char lookAhead(std::size_t from) const noexcept;

        // Строит лексему и проверяет её; при ошибке — XPATH_ERROR с позицией
        Token makeToken(TokenType type, std::string text) const;

        [[noreturn]] void fail(const std::string& message) const;

        std::string_view _source;
        std::size_t _position;
        std::vector<Token> _tokens;
    };
}

#endif
