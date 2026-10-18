#include "xsd/internal/path_tracker.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    // Нулевой уровень — сам документ: его единственный ребёнок — корень
    // The zero level is the document itself: its only child is the root
    PathTracker::PathTracker()
        : _levels(1), _depth(1), _steps(), _names(), _nameIndex()
    {
    }

    /*
    ** Номер среди соседей берётся у родителя до того, как массив уровней
    ** может вырасти (рост переносит уровни, и ссылка на родителя устарела бы).
    ** Хеш-таблица прошлого владельца уровня заменяется новой, а не очищается:
    ** очистка таблицы с огромным числом корзин стоила бы времени на каждом
    ** следующем элементе этого уровня.
    **
    ** The number among siblings is taken from the parent before the level array
    ** may grow (growth moves the levels, and the reference to the parent would become stale).
    ** The hash table of the previous owner of the level is replaced with a new one, not cleared:
    ** clearing a table with a huge number of buckets would cost time on every
    ** next element of this level.
    */
    void PathTracker::open(std::string_view name)
    {
        std::size_t ordinal = countChild(_levels[_depth - 1], name);

        if (_depth == _levels.size())
            _levels.emplace_back();
        PathLevel& level = _levels[_depth];
        level.name.assign(name.data(), name.size());
        level.ordinal = ordinal;
        level.step = NO_STEP;
        level.fewUsed = 0;
        level.lastHit = 0;
        level.pinnedChildren.clear();
        if (!level.many.empty())
        {
            std::unordered_map<std::string_view, std::size_t, utils::SeededHash> fresh;
            level.many.swap(fresh);
        }
        ++_depth;
    }

    // Все дети закрываемого элемента уже пришли: номер нужен, если имя встретилось больше раза
    // All children of the closed element have already arrived: a number is needed if the name occurred more than once
    void PathTracker::close()
    {
        PathLevel& level = _levels[_depth - 1];

        for (std::size_t child : level.pinnedChildren)
            _steps[child].numbered = countOf(level, _steps[child].name) > 1;
        --_depth;
    }

    /*
    ** Закрепление: если элемент закреплён, его предки закреплены тоже,
    ** поэтому ищется самый верхний незакреплённый уровень цепочки, и шаги
    ** создаются от него вниз. Каждый шаг регистрируется у родителя, чтобы
    ** при закрытии родителя узнать, нужен ли ему номер.
    **
    ** Pinning: if an element is pinned, its ancestors are pinned too,
    ** so the topmost unpinned level of the chain is found, and the steps
    ** are created from it downwards. Each step is registered at the parent so that
    ** on closing the parent it becomes known whether the step needs a number.
    */
    std::size_t PathTracker::pin(std::size_t level)
    {
        std::size_t index = level + 1;
        std::size_t first = index;

        if (_levels[index].step != NO_STEP)
            return _levels[index].step;
        while (first > 1 && _levels[first - 1].step == NO_STEP)
            --first;
        for (std::size_t current = first; current <= index; ++current)
        {
            std::size_t parent = NO_STEP;
            if (current > 1)
                parent = _levels[current - 1].step;
            _steps.push_back(PathStep{parent, intern(_levels[current].name), _levels[current].ordinal, false});
            _levels[current].step = _steps.size() - 1;
            _levels[current - 1].pinnedChildren.push_back(_levels[current].step);
        }
        return _levels[index].step;
    }

    // Шаги собираются от элемента к корню, текст — от корня к элементу
    // The steps are collected from the element to the root, the text from the root to the element
    std::string PathTracker::text(std::size_t step) const
    {
        std::vector<std::size_t> chain;
        std::string path;

        for (std::size_t current = step; current != NO_STEP; current = _steps[current].parent)
            chain.push_back(current);
        for (auto position = chain.rbegin(); position != chain.rend(); ++position)
        {
            const PathStep& segment = _steps[*position];
            path += '/';
            path += segment.name;
            if (segment.numbered)
                path += "[" + std::to_string(segment.ordinal) + "]";
        }
        return path;
    }
}
