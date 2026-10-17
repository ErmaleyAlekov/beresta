#include "xsd/regex/xsd_regex.hpp"

#include <utility>

namespace beresta::xsd
{
    /*
    ** Простой атом: '.' — любой символ, кроме переводов строк;
    ** '\' — экранирование (одиночный символ или класс); иначе — сам символ.
    */
    RegexNode RegexParser::parseSimpleAtom()
    {
        CharSet set;
        bool single = true;
        std::uint32_t value = peekCode();

        if (value == '\\')
        {
            set = parseEscape(single, value);
            if (single)
                set.add(value, value);
            return makeSetNode(set);
        }
        takeCode();
        if (value != '.')
        {
            set.add(value, value);
            return makeSetNode(set);
        }
        set.add('\n', '\n');
        set.add('\r', '\r');
        return makeSetNode(set.complement());
    }

    // ? * + или {...}; отсутствие квантификатора — ровно один раз
    void RegexParser::parseQuantifier(RegexNode& node)
    {
        if (atEnd())
            return;
        std::uint32_t code = peekCode();
        if (code == '{')
        {
            parseBraceQuantifier(node);
            return;
        }
        if (code != '?' && code != '*' && code != '+')
            return;
        takeCode();
        node.minimum = 1;
        if (code != '+')
            node.minimum = 0;
        node.maximum = REGEX_UNBOUNDED;
        if (code == '?')
            node.maximum = 1;
    }

    // {n} {n,} {n,m}
    void RegexParser::parseBraceQuantifier(RegexNode& node)
    {
        takeCode();
        node.minimum = parseNumber();
        node.maximum = node.minimum;
        if (!atEnd() && peekCode() == ',')
        {
            takeCode();
            node.maximum = REGEX_UNBOUNDED;
            if (!atEnd() && peekCode() != '}')
                node.maximum = parseNumber();
        }
        if (atEnd() || takeCode() != '}' || node.maximum < node.minimum)
            fail("некорректный квантификатор {n,m}");
    }

    // Неотрицательное десятичное число в квантификаторе (с защитой от переполнения)
    std::size_t RegexParser::parseNumber()
    {
        std::size_t value = 0;
        bool digits = false;

        while (!atEnd() && peekCode() >= '0' && peekCode() <= '9')
        {
            value = value * 10 + (takeCode() - '0');
            digits = true;
            if (value > 100000)
                fail("слишком большое число в квантификаторе");
        }
        if (!digits)
            fail("ожидалось число в квантификаторе");
        return value;
    }
}
