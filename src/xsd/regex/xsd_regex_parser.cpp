#include "xsd/regex/xsd_regex.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xsd
{
    // Множества символов складываются в общий список автомата
    // Character sets are put into the common automaton list
    RegexParser::RegexParser(std::string_view pattern, std::vector<CharSet>& sets)
        : _pattern(pattern), _position(0), _sets(sets)
    {
    }

    // Всё выражение; непарная ')' в конце — ошибка
    // The whole expression; an unpaired ')' at the end is an error
    RegexNode RegexParser::parse()
    {
        RegexNode root = parseAlternation(0);

        if (!atEnd())
            fail(i18n::format(i18n::Message::PATTERN_EXTRA_PARENTHESIS));
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
    // piece* up to '|' or ')'; an empty branch matches the empty string
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
    **
    ** atom: a group (...), a class [...] or a simple atom (a character, '.', an escape).
    ** The metacharacters ?*+{}] without an escape in the place of an atom are an error.
    */
    RegexNode RegexParser::parseAtom(std::size_t depth)
    {
        std::uint32_t code = peekCode();

        if (code == '(')
        {
            if (depth > 64)
                fail(i18n::format(i18n::Message::PATTERN_GROUPS_TOO_DEEP));
            takeCode();
            RegexNode group = parseAlternation(depth + 1);
            if (atEnd() || takeCode() != ')')
                fail(i18n::format(i18n::Message::PATTERN_GROUP_UNCLOSED));
            return group;
        }
        if (code == '[')
            return makeSetNode(parseClassExpression());
        if (code == '?' || code == '*' || code == '+' || code == '{' || code == '}' || code == ']')
            fail(i18n::format(i18n::Message::PATTERN_UNEXPECTED_META));
        return parseSimpleAtom();
    }
}
