#include "xsd/regex/xsd_regex.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd
{
    // \n \r \t — управляющие символы; остальные одиночные экранирования — сам символ
    // \n \r \t are control characters; other single escapes are the character itself
    static std::uint32_t singleEscapeValue(std::uint32_t code) noexcept
    {
        if (code == 'n')
            return '\n';
        if (code == 'r')
            return '\r';
        if (code == 't')
            return '\t';
        return code;
    }

    // Многосимвольные экранирования: \s \d \w \i \c и их дополнения прописными буквами
    // Multi-character escapes: \s \d \w \i \c and their complements in upper case
    static bool isMultiCharEscape(std::uint32_t code) noexcept
    {
        std::string_view multi = "sSdDwWiIcC";

        return code < 0x80 && multi.find(static_cast<char>(code)) != std::string_view::npos;
    }

    /*
    ** Экранирование: одиночный символ возвращается через single, а
    ** многосимвольное и свойства \p{..} / \P{..} — как множество.
    **
    ** An escape: a single character is returned through single, and
    ** a multi-character one and the \p{..} / \P{..} properties as a set.
    */
    CharSet RegexParser::parseEscape(bool& isSingle, std::uint32_t& single)
    {
        takeCode();
        if (atEnd())
            fail(i18n::format(i18n::Message::PATTERN_UNFINISHED_ESCAPE));
        std::uint32_t code = takeCode();
        isSingle = !isMultiCharEscape(code) && code != 'p' && code != 'P';
        if (code == 'p' || code == 'P')
            return parseProperty(code == 'P');
        if (isSingle)
        {
            single = singleEscapeValue(code);
            return CharSet();
        }
        CharSet set = multiCharEscapeSet(static_cast<char>(code | 0x20));
        if (code < 'a')
            return set.complement();
        return set;
    }

    // \p{Имя} — категория или блок; \P{Имя} — дополнение
    // \p{Name} — a category or a block; \P{Name} — the complement
    CharSet RegexParser::parseProperty(bool complement)
    {
        std::size_t close = _pattern.find('}', _position);

        if (atEnd() || peekCode() != '{' || close == std::string_view::npos)
            fail(i18n::format(i18n::Message::PATTERN_PROPERTY_SYNTAX));
        std::string_view name = _pattern.substr(_position + 1, close - _position - 1);
        _position = close + 1;
        bool known = false;
        CharSet set = unicodePropertySet(name, known);
        if (!known)
            fail(i18n::format(i18n::Message::PATTERN_UNKNOWN_PROPERTY, {name}));
        if (complement)
            return set.complement();
        return set;
    }
}
