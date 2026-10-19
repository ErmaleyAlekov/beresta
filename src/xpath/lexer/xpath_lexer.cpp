#include "xpath/internal/xpath_lexer.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::xpath::detail
{
    // Лексер работает по представлению исходной строки в заданном диалекте
    // The lexer works over a view of the source string in the given dialect
    XPathLexer::XPathLexer(std::string_view source, XPathDialect dialect) noexcept
        : _source(source), _dialect(dialect), _position(0), _tokens()
    {
    }

    // Лексемы читаются до конца; пробелы между ними игнорируются
    // Tokens are read to the end; spaces between them are ignored
    std::vector<Token> XPathLexer::tokenize()
    {
        while (true)
        {
            while (_position < _source.size() && utils::isWhitespace(_source[_position]))
                ++_position;
            if (_position >= _source.size())
                break;
            _tokens.push_back(readToken());
        }
        _tokens.push_back(makeToken(TokenType::END_OF_INPUT, ""));
        return _tokens;
    }

    // Выбор вида лексемы по первому символу
    // Chooses the token kind by the first character
    Token XPathLexer::readToken()
    {
        char symbol = _source[_position];
        char next = '\0';

        if (_position + 1 < _source.size())
            next = _source[_position + 1];
        if (symbol >= '0' && symbol <= '9')
            return readNumber();
        if (symbol == '.' && next >= '0' && next <= '9')
            return readNumber();
        if (symbol == '"' || symbol == '\'')
            return readLiteral();
        if (symbol == '*' || utils::isNameStartChar(symbol))
        {
            if (symbol != ':')
                return readNameLike();
        }
        return readSymbol();
    }

    // Число: цифры с необязательной дробной частью (экспоненты в XPath 1.0 нет)
    // A number: digits with an optional fraction (XPath 1.0 has no exponent)
    Token XPathLexer::readNumber()
    {
        std::size_t start = _position;
        long double value = 0.0L;
        long double scale = 1.0L;
        bool fraction = false;

        while (_position < _source.size())
        {
            char symbol = _source[_position];
            if (symbol == '.' && !fraction)
                fraction = true;
            else if (symbol >= '0' && symbol <= '9')
            {
                value = value * 10.0L + (symbol - '0');
                if (fraction)
                    scale /= 10.0L;
            }
            else
                break;
            ++_position;
        }
        Token token = makeToken(TokenType::NUMBER, std::string(_source.substr(start, _position - start)));
        token.number = static_cast<double>(value * scale);
        return token;
    }

    // Строковый литерал в одинарных или двойных кавычках (экранирования в XPath 1.0 нет)
    // A string literal in single or double quotes (XPath 1.0 has no escaping)
    Token XPathLexer::readLiteral()
    {
        char quote = _source[_position];
        std::size_t end = _source.find(quote, _position + 1);

        if (end == std::string_view::npos)
            fail(i18n::format(i18n::Message::XPATH_UNCLOSED_LITERAL));
        std::string text(_source.substr(_position + 1, end - _position - 1));
        _position = end + 1;
        return makeToken(TokenType::LITERAL, text);
    }
}
