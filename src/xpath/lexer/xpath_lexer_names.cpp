#include "xpath/internal/xpath_lexer.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::xpath::detail
{
    // Типы узлов, которые записываются как вызов: node(), text() и т.д.
    // Node types written as a call: node(), text() and so on
    static bool isNodeTypeName(const std::string& name) noexcept
    {
        if (name == "node" || name == "text")
            return true;
        return name == "comment" || name == "processing-instruction";
    }

    /*
    ** Имя или '*'. Правила однозначности XPath 1.0: после операнда '*' —
    ** умножение, а имя — оператор (and, or, mod, div).
    **
    ** A name or '*'. The XPath 1.0 disambiguation rules: after an operand '*' is
    ** multiplication, and a name is an operator (and, or, mod, div).
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
        fail(i18n::format(i18n::Message::XPATH_EXPECTED_OPERATOR, {name}));
    }

    /*
    ** Префикс "p:" (но не ось "p::"), затем классификация: имя перед "::" —
    ** ось, перед '(' — тип узла или функция, иначе — проверка имени.
    **
    ** The prefix "p:" (but not the axis "p::"), then the classification: a name before "::" is
    ** an axis, before '(' — a node type or a function, otherwise — a name test.
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

    /*
    ** NCName — имя XML без двоеточий. Символы проверяются по таблицам
    ** XML 1.0 5-й редакции (как у парсера), с учётом многобайтового UTF-8:
    ** иначе выражение могло бы принять имя, которого нет ни в одном документе.
    **
    ** NCName — an XML name without colons. The characters are checked against the
    ** XML 1.0 5th edition tables (as in the parser), taking multi-byte UTF-8 into account:
    ** otherwise an expression could accept a name that exists in no document.
    */
    std::string XPathLexer::readNcName()
    {
        std::size_t start = _position;
        std::size_t length = 0;

        if (_position < _source.size() && _source[_position] != ':')
            length = utils::nameCharLength(_source, _position, true);
        if (length == 0)
            fail(i18n::format(i18n::Message::XPATH_EXPECTED_NAME));
        while (length > 0)
        {
            _position += length;
            length = 0;
            if (_position < _source.size() && _source[_position] != ':')
                length = utils::nameCharLength(_source, _position, false);
        }
        return std::string(_source.substr(start, _position - start));
    }

    // Подсматривание через пробелы (для распознавания '(' и "::" после имени)
    // Peeking through spaces (to recognize '(' and "::" after a name)
    char XPathLexer::lookAhead(std::size_t from) const noexcept
    {
        while (from < _source.size() && utils::isWhitespace(_source[from]))
            ++from;
        if (from >= _source.size())
            return '\0';
        return _source[from];
    }
}
