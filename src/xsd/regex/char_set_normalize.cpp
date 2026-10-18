#include "xsd/regex/char_set.hpp"

#include <algorithm>

namespace beresta::xsd
{
    // Сортировка по началу и слияние пересекающихся или смежных диапазонов
    // Sorts by the start and merges overlapping or adjacent ranges
    void CharSet::normalize()
    {
        std::vector<Range> merged;

        std::sort(_ranges.begin(), _ranges.end());
        for (const Range& range : _ranges)
        {
            bool touches = !merged.empty() && (merged.back().second >= MAX_CODE_POINT
                || range.first <= merged.back().second + 1);
            if (touches)
                merged.back().second = std::max(merged.back().second, range.second);
            else
                merged.push_back(range);
        }
        _ranges = std::move(merged);
    }

    // Диапазоны для чтения
    // The ranges for reading
    const std::vector<CharSet::Range>& CharSet::getRanges() const noexcept
    {
        return _ranges;
    }
}
