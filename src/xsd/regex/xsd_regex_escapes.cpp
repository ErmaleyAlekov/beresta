#include "xsd/regex/xsd_regex.hpp"

namespace beresta::xsd
{
    // \n \r \t — управляющие символы; остальные одиночные экранирования — сам символ
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
    static bool isMultiCharEscape(std::uint32_t code) noexcept
    {
        std::string_view multi = "sSdDwWiIcC";

        return code < 0x80 && multi.find(static_cast<char>(code)) != std::string_view::npos;
    }

    /*
    ** Экранирование: одиночный символ возвращается через single, а
    ** многосимвольное и свойства \p{..} / \P{..} — как множество.
    */
    CharSet RegexParser::parseEscape(bool& isSingle, std::uint32_t& single)
    {
        takeCode();
        if (atEnd())
            fail("незавершённое экранирование");
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
    CharSet RegexParser::parseProperty(bool complement)
    {
        std::size_t close = _pattern.find('}', _position);

        if (atEnd() || peekCode() != '{' || close == std::string_view::npos)
            fail("ожидалось \\p{имя}");
        std::string_view name = _pattern.substr(_position + 1, close - _position - 1);
        _position = close + 1;
        bool known = false;
        CharSet set = unicodePropertySet(name, known);
        if (!known)
            fail("неизвестное свойство Unicode \\p{" + std::string(name) + "}");
        if (complement)
            return set.complement();
        return set;
    }
}
