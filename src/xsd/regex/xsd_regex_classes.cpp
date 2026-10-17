#include "xsd/regex/xsd_regex.hpp"

namespace beresta::xsd
{
    // '[' группа ']'
    CharSet RegexParser::parseClassExpression()
    {
        takeCode();
        CharSet set = parseClassGroup();
        if (atEnd() || takeCode() != ']')
            fail("не закрыт класс символов '['");
        return set;
    }

    /*
    ** Группа: [^...] — отрицание, [...-[...]] — вычитание вложенного класса.
    ** Вычитание применяется после отрицания, как в спецификации.
    */
    CharSet RegexParser::parseClassGroup()
    {
        CharSet set;
        bool negated = false;

        if (!atEnd() && peekCode() == '^')
        {
            takeCode();
            negated = true;
        }
        while (!atEnd() && peekCode() != ']')
        {
            if (applySubtraction(set, negated))
                return set;
            parseClassItem(set);
        }
        set.normalize();
        if (negated)
            return set.complement();
        return set;
    }

    // "-[" начинает вычитание: (группа с учётом отрицания) минус вложенный класс
    bool RegexParser::applySubtraction(CharSet& set, bool negated)
    {
        bool subtraction = peekCode() == '-' && _position + 1 < _pattern.size()
            && _pattern[_position + 1] == '[';

        if (!subtraction)
            return false;
        takeCode();
        CharSet removed = parseClassExpression();
        set.normalize();
        if (negated)
            set = set.complement();
        set = set.subtract(removed);
        return true;
    }

    // Элемент класса: экранирование, символ или диапазон a-z
    void RegexParser::parseClassItem(CharSet& set)
    {
        bool single = true;
        std::uint32_t first = 0;

        if (peekCode() == '\\')
        {
            CharSet escaped = parseEscape(single, first);
            if (!single)
            {
                set.addSet(escaped);
                return;
            }
        }
        else
            first = takeCode();
        bool range = !atEnd() && peekCode() == '-' && _position + 1 < _pattern.size()
            && _pattern[_position + 1] != '[' && _pattern[_position + 1] != ']';
        if (!range)
        {
            set.add(first, first);
            return;
        }
        takeCode();
        set.add(first, parseRangeEnd());
    }

    // Конец диапазона: символ или одиночное экранирование (\- , \] и т.п.)
    std::uint32_t RegexParser::parseRangeEnd()
    {
        bool single = true;
        std::uint32_t last = 0;

        if (peekCode() != '\\')
            return takeCode();
        parseEscape(single, last);
        if (!single)
            fail("конец диапазона не может быть классом символов");
        return last;
    }
}
