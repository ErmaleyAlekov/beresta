#ifndef CHAR_SET_HPP
#define CHAR_SET_HPP

#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

namespace beresta::xsd
{
    /*
    ** CharSet — множество кодовых точек Unicode, хранимое как упорядоченный
    ** список непересекающихся диапазонов [first, last]. Все операции
    ** регулярных выражений XSD над классами символов (объединение,
    ** дополнение [^...], вычитание [a-z-[aeiou]]) сводятся к операциям
    ** над диапазонами, а проверка принадлежности — двоичный поиск.
    **
    ** CharSet — a set of Unicode code points stored as an ordered
    ** list of disjoint ranges [first, last]. All XSD regular expression
    ** operations on character classes (union, complement [^...],
    ** subtraction [a-z-[aeiou]]) reduce to operations on ranges,
    ** and the membership test is a binary search.
    */
    class CharSet
    {
    public:
        using Range = std::pair<std::uint32_t, std::uint32_t>;

        // Добавление диапазона и целого множества
        // Adds a range and a whole set
        void add(std::uint32_t first, std::uint32_t last);
        void addSet(const CharSet& other);

        // Дополнение до всего диапазона Unicode и разность множеств
        // The complement to the whole Unicode range and the set difference
        CharSet complement() const;
        CharSet subtract(const CharSet& other) const;

        // Принадлежность кодовой точки (после normalize())
        // Membership of a code point (after normalize())
        bool contains(std::uint32_t code) const noexcept;

        // Сортировка и слияние пересекающихся диапазонов
        // Sorts and merges overlapping ranges
        void normalize();

        const std::vector<Range>& getRanges() const noexcept;

    private:
        std::vector<Range> _ranges;
    };

    // Последний код Unicode
    // The last Unicode code
    inline constexpr std::uint32_t MAX_CODE_POINT = 0x10FFFF;

    // Множество для категории \p{...} или блока \p{Is...}; known = false — имя неизвестно
    // The set for the \p{...} category or the \p{Is...} block; known = false — the name is unknown
    CharSet unicodePropertySet(std::string_view name, bool& known);

    // Множество для многосимвольного экранирования \s \i \c \d \w (строчные буквы)
    // The set for the multi-character escape \s \i \c \d \w (lower-case letters)
    CharSet multiCharEscapeSet(char letter);
}

#endif
