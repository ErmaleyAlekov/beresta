#include "xpath/internal/xpath_lexer.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Двухсимвольные лексемы проверяются раньше односимвольных
    struct SymbolToken
    {
        const char* text;
        TokenType type;
    };

    static const SymbolToken SYMBOLS[] = {
        {"//", TokenType::DOUBLE_SLASH}, {"!=", TokenType::NOT_EQUAL}, {"<=", TokenType::LESS_EQUAL},
        {">=", TokenType::GREATER_EQUAL}, {"..", TokenType::DOUBLE_DOT}, {"::", TokenType::DOUBLE_COLON},
        {"/", TokenType::SLASH}, {"|", TokenType::PIPE}, {"+", TokenType::PLUS},
        {"-", TokenType::MINUS_SIGN}, {"=", TokenType::EQUAL}, {"<", TokenType::LESS},
        {">", TokenType::GREATER}, {"(", TokenType::LEFT_PAREN}, {")", TokenType::RIGHT_PAREN},
        {"[", TokenType::LEFT_BRACKET}, {"]", TokenType::RIGHT_BRACKET}, {".", TokenType::DOT},
        {"@", TokenType::AT_SIGN}, {",", TokenType::COMMA}
    };

    // Знаки операций, скобки и переменные ($имя)
    Token XPathLexer::readSymbol()
    {
        if (_source[_position] == '$')
        {
            ++_position;
            std::string name = readNcName();
            if (_position + 1 < _source.size() && _source[_position] == ':' && _source[_position + 1] != ':')
            {
                ++_position;
                name += ":" + readNcName();
            }
            return makeToken(TokenType::VARIABLE, name);
        }
        std::string_view rest = _source.substr(_position);
        for (const SymbolToken& symbol : SYMBOLS)
        {
            std::string_view text(symbol.text);
            if (rest.substr(0, text.size()) != text)
                continue;
            _position += text.size();
            return makeToken(symbol.type, std::string(text));
        }
        fail("неожиданный символ '" + std::string(1, _source[_position]) + "'");
    }

    /*
    ** Предыдущая лексема — операнд, если она не '@', '::', '(', '[', ','
    ** и не оператор. Тогда следующий '*' — умножение, а имя — оператор.
    */
    bool XPathLexer::operatorExpected() const noexcept
    {
        if (_tokens.empty())
            return false;
        switch (_tokens.back().type)
        {
            case TokenType::AT_SIGN: case TokenType::DOUBLE_COLON: case TokenType::LEFT_PAREN:
            case TokenType::LEFT_BRACKET: case TokenType::COMMA: case TokenType::OPERATOR_NAME:
            case TokenType::MULTIPLY: case TokenType::SLASH: case TokenType::DOUBLE_SLASH:
            case TokenType::PIPE: case TokenType::PLUS: case TokenType::MINUS_SIGN:
            case TokenType::EQUAL: case TokenType::NOT_EQUAL: case TokenType::LESS:
            case TokenType::LESS_EQUAL: case TokenType::GREATER: case TokenType::GREATER_EQUAL:
                return false;
            default:
                return true;
        }
    }

    // Лексема с нулевым числовым значением
    Token XPathLexer::makeToken(TokenType type, std::string text) const
    {
        return Token{type, std::move(text), 0.0};
    }

    // Синтаксическая ошибка с позицией в выражении (нумерация символов с единицы)
    void XPathLexer::fail(const std::string& message) const
    {
        throw DomException(DomErrorCode::XPATH_ERROR, "XPath, позиция "
            + std::to_string(_position + 1) + ": " + message + " в \"" + std::string(_source) + "\"");
    }
}
