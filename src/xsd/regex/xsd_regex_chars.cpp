#include "xsd/regex/xsd_regex.hpp"
#include "exception/dom_exception.hpp"
#include "utils/utf8.hpp"

#include <utility>

namespace beresta::xsd
{
    // Шаблон прочитан до конца
    bool RegexParser::atEnd() const noexcept
    {
        return _position >= _pattern.size();
    }

    // Текущий символ шаблона (кодовая точка UTF-8) без сдвига
    std::uint32_t RegexParser::peekCode() const noexcept
    {
        std::size_t index = _position;
        std::uint32_t code = 0;

        if (atEnd() || !utils::decodeUtf8Char(_pattern, index, code))
            return 0;
        return code;
    }

    // Текущий символ шаблона со сдвигом
    std::uint32_t RegexParser::takeCode()
    {
        std::uint32_t code = 0;

        if (atEnd() || !utils::decodeUtf8Char(_pattern, _position, code))
            fail("неожиданный конец или некорректный символ в шаблоне");
        return code;
    }

    // Узел «один символ из множества»; множество уходит в общий список автомата
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
    void RegexParser::fail(const std::string& message) const
    {
        throw DomException(DomErrorCode::SCHEMA_ERROR, "шаблон \"" + std::string(_pattern)
            + "\", позиция " + std::to_string(_position + 1) + ": " + message);
    }
}
