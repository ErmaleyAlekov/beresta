#include "xsd/regex/xsd_regex.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/utf8.hpp"

#include <utility>

namespace beresta::xsd
{
    // Шаблон прочитан до конца
    // The pattern is read to the end
    bool RegexParser::atEnd() const noexcept
    {
        return _position >= _pattern.size();
    }

    // Текущий символ шаблона (кодовая точка UTF-8) без сдвига
    // The current pattern character (a UTF-8 code point) without moving
    std::uint32_t RegexParser::peekCode() const noexcept
    {
        std::size_t index = _position;
        std::uint32_t code = 0;

        if (atEnd() || !utils::decodeUtf8Char(_pattern, index, code))
            return 0;
        return code;
    }

    // Текущий символ шаблона со сдвигом
    // The current pattern character with moving
    std::uint32_t RegexParser::takeCode()
    {
        std::uint32_t code = 0;

        if (atEnd() || !utils::decodeUtf8Char(_pattern, _position, code))
            fail(i18n::format(i18n::Message::PATTERN_BAD_CHARACTER));
        return code;
    }

    // Узел «один символ из множества»; множество уходит в общий список автомата
    // The "one character from a set" node; the set goes into the common automaton list
    RegexNode RegexParser::makeSetNode(CharSet set)
    {
        RegexNode node;

        set.normalize();
        node.kind = RegexKind::CHARACTERS;
        node.setIndex = _sets.size();
        _sets.push_back(std::move(set));
        return node;
    }

    // Ошибка шаблона указывает позицию в байтах (с единицы)
    // A pattern error gives the position in bytes (from one)
    void RegexParser::fail(const std::string& message) const
    {
        throw DomException(DomErrorCode::SCHEMA_ERROR, i18n::format(i18n::Message::PATTERN_AT_POSITION, {_pattern, std::to_string(_position + 1), message}));
    }
}
