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
    */
    class CharSet
    {
    public:
        using Range = std::pair<std::uint32_t, std::uint32_t>;

        // Добавление диапазона и целого множества
        void add(std::uint32_t first, std::uint32_t last);
        void addSet(const CharSet& other);

        // Дополнение до всего диапазона Unicode и разность множеств
        CharSet complement() const;
        CharSet subtract(const CharSet& other) const;

        // Принадлежность кодовой точки (после normalize())
        bool contains(std::uint32_t code) const noexcept;

        // Сортировка и слияние пересекающихся диапазонов
        void normalize();

        const std::vector<Range>& getRanges() const noexcept;

    private:
        std::vector<Range> _ranges;
    };

    // Последний код Unicode
    inline constexpr std::uint32_t MAX_CODE_POINT = 0x10FFFF;

    // Множество для категории \p{...} или блока \p{Is...}; known = false — имя неизвестно
    CharSet unicodePropertySet(std::string_view name, bool& known);

    // Множество для многосимвольного экранирования \s \i \c \d \w (строчные буквы)
    CharSet multiCharEscapeSet(char letter);
}

#endif
