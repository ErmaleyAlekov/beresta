#include "xsd/regex/xsd_regex.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xsd
{
    /*
    ** Простой атом: '.' — любой символ, кроме переводов строк;
    ** '\' — экранирование (одиночный символ или класс); иначе — сам символ.
    **
    ** A simple atom: '.' — any character except line breaks;
    ** '\' — an escape (a single character or a class); otherwise — the character itself.
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
    // ? * + or {...}; no quantifier means exactly once
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
            fail(i18n::format(i18n::Message::PATTERN_QUANTIFIER_INVALID));
    }

    // Неотрицательное десятичное число в квантификаторе (с защитой от переполнения)
    // A non-negative decimal number in a quantifier (with overflow protection)
    std::size_t RegexParser::parseNumber()
    {
        std::size_t value = 0;
        bool digits = false;

        while (!atEnd() && peekCode() >= '0' && peekCode() <= '9')
        {
            value = value * 10 + (takeCode() - '0');
            digits = true;
            if (value > 100000)
                fail(i18n::format(i18n::Message::PATTERN_QUANTIFIER_TOO_LARGE));
        }
        if (!digits)
            fail(i18n::format(i18n::Message::PATTERN_QUANTIFIER_NUMBER));
        return value;
    }
}
