#include "xsd/internal/path_tracker.hpp"

namespace beresta::xsd::detail
{
    // Разных имён детей не больше этого — счётчики в массиве, дальше — в хеш-таблице
    // No more different child names than this — the counters are in an array, beyond that — in a hash table
    static const std::size_t FEW_NAMES = 8;

    // Номер счётчика имени в массиве или fewUsed, если имени там нет (сначала — последний найденный)
    // The index of the name counter in the array or fewUsed if the name is not there (the last found one first)
    static std::size_t findFew(const PathLevel& level, std::string_view name) noexcept
    {
        if (level.lastHit < level.fewUsed && level.few[level.lastHit].name == name)
            return level.lastHit;
        for (std::size_t index = 0; index < level.fewUsed; ++index)
        {
            if (level.few[index].name == name)
                return index;
        }
        return level.fewUsed;
    }

    /*
    ** Счёт очередного ребёнка с именем name; возвращается его номер среди
    ** одноимённых соседей. Обычно имён мало и подряд идут одинаковые
    ** (item, item, item…) — тогда хватает одного сравнения с последним
    ** найденным. Девятое разное имя переводит уровень на хеш-таблицу:
    ** перебор у элемента со 100 тысячами разных детей был бы квадратичным.
    **
    ** Counts the next child with the name name; its number among same-named
    ** siblings is returned. Usually there are few names and equal ones come in a row
    ** (item, item, item…) — then one comparison with the last found one
    ** is enough. The ninth different name switches the level to a hash table:
    ** a loop for an element with 100 thousand different children would be quadratic.
    */
    std::size_t PathTracker::countChild(PathLevel& parent, std::string_view name)
    {
        if (!parent.many.empty())
            return countMany(parent, name);
        std::size_t index = findFew(parent, name);
        if (index < parent.fewUsed)
        {
            parent.lastHit = index;
            return ++parent.few[index].count;
        }
        if (parent.fewUsed == FEW_NAMES)
        {
            for (std::size_t old = 0; old < parent.fewUsed; ++old)
                parent.many.emplace(intern(parent.few[old].name), parent.few[old].count);
            return countMany(parent, name);
        }
        if (parent.fewUsed == parent.few.size())
            parent.few.emplace_back();
        parent.few[parent.fewUsed].name.assign(name.data(), name.size());
        parent.few[parent.fewUsed].count = 1;
        parent.lastHit = parent.fewUsed++;
        return 1;
    }

    // Счёт в хеш-таблице: имя копируется в общий пул, только если встретилось впервые
    // Counting in the hash table: the name is copied into the common pool only if it occurs for the first time
    std::size_t PathTracker::countMany(PathLevel& parent, std::string_view name)
    {
        auto found = parent.many.find(name);

        if (found != parent.many.end())
            return ++found->second;
        parent.many.emplace(intern(name), 1);
        return 1;
    }

    // Сколько детей с именем name насчитано у уровня
    // How many children with the name name the level has counted
    std::size_t PathTracker::countOf(const PathLevel& parent, std::string_view name) const
    {
        if (!parent.many.empty())
        {
            auto found = parent.many.find(name);
            if (found == parent.many.end())
                return 0;
            return found->second;
        }
        std::size_t index = findFew(parent, name);
        if (index < parent.fewUsed)
            return parent.few[index].count;
        return 0;
    }

    /*
    ** Единственная копия имени на весь проход: шаги путей и ключи
    ** хеш-таблиц ссылаются на неё представлениями. deque не переносит
    ** строки при росте, поэтому представления остаются действительными.
    **
    ** The single copy of a name for the whole pass: the path steps and the keys
    ** of the hash tables refer to it by views. A deque does not move
    ** strings on growth, so the views stay valid.
    */
    std::string_view PathTracker::intern(std::string_view name)
    {
        auto found = _nameIndex.find(name);

        if (found != _nameIndex.end())
            return found->second;
        _names.emplace_back(name.data(), name.size());
        std::string_view stored = _names.back();
        _nameIndex.emplace(stored, stored);
        return stored;
    }
}
