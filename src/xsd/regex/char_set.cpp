#include "xsd/regex/char_set.hpp"

#include <algorithm>

namespace beresta::xsd
{
    // Диапазон добавляется как есть; порядок восстанавливает normalize()
    // A range is added as is; normalize() restores the order
    void CharSet::add(std::uint32_t first, std::uint32_t last)
    {
        if (first > last)
            std::swap(first, last);
        _ranges.emplace_back(first, last);
    }

    // Объединение — просто дописывание диапазонов другого множества
    // A union is simply appending the ranges of the other set
    void CharSet::addSet(const CharSet& other)
    {
        _ranges.insert(_ranges.end(), other._ranges.begin(), other._ranges.end());
    }

    // Дополнение: промежутки между диапазонами нормализованного множества
    // The complement: the gaps between the ranges of the normalized set
    CharSet CharSet::complement() const
    {
        CharSet sorted = *this;
        CharSet result;
        std::uint32_t next = 0;

        sorted.normalize();
        for (const Range& range : sorted._ranges)
        {
            if (range.first > next)
                result._ranges.emplace_back(next, range.first - 1);
            if (range.second >= MAX_CODE_POINT)
                return result;
            next = range.second + 1;
        }
        if (next <= MAX_CODE_POINT)
            result._ranges.emplace_back(next, MAX_CODE_POINT);
        return result;
    }

    // Разность A \ B = A ∩ дополнение(B): пересекаем каждую пару диапазонов
    // The difference A \ B = A ∩ complement(B): each pair of ranges is intersected
    CharSet CharSet::subtract(const CharSet& other) const
    {
        CharSet allowed = other.complement();
        CharSet self = *this;
        CharSet result;

        self.normalize();
        for (const Range& left : self._ranges)
        {
            for (const Range& right : allowed._ranges)
            {
                std::uint32_t first = std::max(left.first, right.first);
                std::uint32_t last = std::min(left.second, right.second);
                if (first <= last)
                    result._ranges.emplace_back(first, last);
            }
        }
        result.normalize();
        return result;
    }

    // Двоичный поиск диапазона, который мог бы содержать код
    // A binary search for the range that could contain the code
    bool CharSet::contains(std::uint32_t code) const noexcept
    {
        std::size_t low = 0;
        std::size_t high = _ranges.size();

        while (low < high)
        {
            std::size_t middle = (low + high) / 2;
            if (_ranges[middle].second < code)
                low = middle + 1;
            else if (_ranges[middle].first > code)
                high = middle;
            else
                return true;
        }
        return false;
    }
}
