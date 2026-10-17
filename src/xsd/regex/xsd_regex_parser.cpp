#include "xsd/regex/xsd_regex.hpp"

#include <utility>

namespace beresta::xsd
{
    // Множества символов складываются в общий список автомата
    RegexParser::RegexParser(std::string_view pattern, std::vector<CharSet>& sets)
        : _pattern(pattern), _position(0), _sets(sets)
    {
    }

    // Всё выражение; непарная ')' в конце — ошибка
    RegexNode RegexParser::parse()
    {
        RegexNode root = parseAlternation(0);

        if (!atEnd())
            fail("лишняя ')' в шаблоне");
        return root;
    }

    // branch ('|' branch)*
    RegexNode RegexParser::parseAlternation(std::size_t depth)
    {
        RegexNode first = parseBranch(depth);

        if (atEnd() || peekCode() != '|')
            return first;
        RegexNode alternation;
        alternation.kind = RegexKind::ALTERNATION;
        alternation.children.push_back(std::move(first));
        while (!atEnd() && peekCode() == '|')
        {
            takeCode();
            alternation.children.push_back(parseBranch(depth));
        }
        return alternation;
    }

    // piece* до '|' или ')'; пустая ветвь совпадает с пустой строкой
    RegexNode RegexParser::parseBranch(std::size_t depth)
    {
        RegexNode branch;

        while (!atEnd() && peekCode() != '|' && peekCode() != ')')
        {
            RegexNode piece = parseAtom(depth);
            parseQuantifier(piece);
            branch.children.push_back(std::move(piece));
        }
        return branch;
    }

    /*
    ** atom: группа (...), класс [...] или простой атом (символ, '.', экранирование).
    ** Метасимволы ?*+{}] без экранирования на месте атома — ошибка.
    */
    RegexNode RegexParser::parseAtom(std::size_t depth)
    {
        std::uint32_t code = peekCode();

        if (code == '(')
        {
            if (depth > 64)
                fail("слишком глубокая вложенность групп");
            takeCode();
            RegexNode group = parseAlternation(depth + 1);
            if (atEnd() || takeCode() != ')')
                fail("не закрыта группа '('");
            return group;
        }
        if (code == '[')
            return makeSetNode(parseClassExpression());
        if (code == '?' || code == '*' || code == '+' || code == '{' || code == '}' || code == ']')
            fail("неожиданный метасимвол в шаблоне");
        return parseSimpleAtom();
    }
}
