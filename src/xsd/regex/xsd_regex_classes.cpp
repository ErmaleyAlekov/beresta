#include "xsd/regex/xsd_regex.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd
{
    // '[' группа ']'
    // '[' group ']'
    CharSet RegexParser::parseClassExpression()
    {
        takeCode();
        CharSet set = parseClassGroup();
        if (atEnd() || takeCode() != ']')
            fail(i18n::format(i18n::Message::PATTERN_CLASS_UNCLOSED));
        return set;
    }

    /*
    ** Группа: [^...] — отрицание, [...-[...]] — вычитание вложенного класса.
    ** Вычитание применяется после отрицания, как в спецификации.
    **
    ** A group: [^...] is a negation, [...-[...]] is a subtraction of a nested class.
    ** The subtraction is applied after the negation, as in the specification.
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
        // Пустой класс [] или [^] грамматика запрещает: posCharGroup — хотя бы один элемент
        // The grammar forbids an empty class [] or [^]: posCharGroup is at least one item
        if (atEnd() || peekCode() == ']')
            fail(i18n::format(i18n::Message::PATTERN_CLASS_EMPTY));
        while (!atEnd() && peekCode() != ']')
        {
            if (applySubtraction(set, negated))
                return set;
            // '[' внутри класса допустим только экранированным (XmlChar) или в вычитании "-["
            // '[' inside a class is allowed only escaped (XmlChar) or in the subtraction "-["
            if (peekCode() == '[')
                fail(i18n::format(i18n::Message::PATTERN_UNEXPECTED_META));
            parseClassItem(set);
        }
        set.normalize();
        if (negated)
            return set.complement();
        return set;
    }

    // "-[" начинает вычитание: (группа с учётом отрицания) минус вложенный класс
    // "-[" starts a subtraction: (the group with the negation applied) minus the nested class
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
    // A class item: an escape, a character or a range a-z
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
        set.add(first, parseRangeEnd(first));
    }

    /*
    ** Конец диапазона: символ или одиночное экранирование (\- , \] и т.п.).
    ** Диапазон [z-a] спецификация запрещает (seRange: начало не больше конца);
    ** раньше его молча переворачивали.
    **
    ** The end of a range: a character or a single escape (\- , \] and so on).
    ** The specification forbids the range [z-a] (seRange: the start is not greater than the end);
    ** it used to be silently reversed.
    */
    std::uint32_t RegexParser::parseRangeEnd(std::uint32_t first)
    {
        bool single = true;
        std::uint32_t last = 0;

        if (peekCode() != '\\')
            last = takeCode();
        else
            parseEscape(single, last);
        if (!single)
            fail(i18n::format(i18n::Message::PATTERN_RANGE_END_CLASS));
        if (last < first)
            fail(i18n::format(i18n::Message::PATTERN_RANGE_REVERSED));
        return last;
    }
}
