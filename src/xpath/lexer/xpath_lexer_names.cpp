#include "xpath/internal/xpath_lexer.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::xpath::detail
{
    // Типы узлов, которые записываются как вызов: node(), text() и т.д.
    static bool isNodeTypeName(const std::string& name) noexcept
    {
        if (name == "node" || name == "text")
            return true;
        return name == "comment" || name == "processing-instruction";
    }

    /*
    ** Имя или '*'. Правила однозначности XPath 1.0: после операнда '*' —
    ** умножение, а имя — оператор (and, or, mod, div).
    */
    Token XPathLexer::readNameLike()
    {
        bool afterOperand = operatorExpected();

        if (_source[_position] == '*')
        {
            ++_position;
            if (afterOperand)
                return makeToken(TokenType::MULTIPLY, "*");
            return makeToken(TokenType::NAME_TEST, "*");
        }
        std::string name = readNcName();
        if (!afterOperand)
            return classifyName(name);
        if (name == "and" || name == "or" || name == "mod" || name == "div")
            return makeToken(TokenType::OPERATOR_NAME, name);
        fail("ожидался оператор, найдено имя \"" + name + "\"");
    }

    /*
    ** Префикс "p:" (но не ось "p::"), затем классификация: имя перед "::" —
    ** ось, перед '(' — тип узла или функция, иначе — проверка имени.
    */
    Token XPathLexer::classifyName(std::string name)
    {
        bool prefixed = _position + 1 < _source.size() && _source[_position] == ':'
            && _source[_position + 1] != ':';

        if (prefixed)
        {
            ++_position;
            if (_position < _source.size() && _source[_position] == '*')
            {
                ++_position;
                return makeToken(TokenType::NAME_TEST, name + ":*");
            }
            name += ":" + readNcName();
        }
        char next = lookAhead(_position);
        if (!prefixed && next == ':' && lookAhead(_source.find(':', _position) + 1) == ':')
            return makeToken(TokenType::AXIS_NAME, name);
        if (next == '(' && !prefixed && isNodeTypeName(name))
            return makeToken(TokenType::NODE_TYPE, name);
        if (next == '(')
            return makeToken(TokenType::FUNCTION_NAME, name);
        return makeToken(TokenType::NAME_TEST, name);
    }

    // NCName — имя XML без двоеточий
    std::string XPathLexer::readNcName()
    {
        std::size_t start = _position;

        if (_position >= _source.size() || !utils::isNameStartChar(_source[_position])
            || _source[_position] == ':')
            fail("ожидалось имя");
        ++_position;
        while (_position < _source.size() && utils::isNameChar(_source[_position])
            && _source[_position] != ':')
            ++_position;
        return std::string(_source.substr(start, _position - start));
    }

    // Подсматривание через пробелы (для распознавания '(' и "::" после имени)
    char XPathLexer::lookAhead(std::size_t from) const noexcept
    {
        while (from < _source.size() && utils::isWhitespace(_source[from]))
            ++from;
        if (from >= _source.size())
            return '\0';
        return _source[from];
    }
}
