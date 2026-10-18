#ifndef XSD_PATH_TRACKER_HPP
#define XSD_PATH_TRACKER_HPP

#include "utils/seeded_hash.hpp"

#include <cstddef>
#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace beresta::xsd::detail
{
    // «Шага нет»: элемент ещё не закреплён или у шага нет родителя
    // "No step": the element is not pinned yet or the step has no parent
    constexpr std::size_t NO_STEP = static_cast<std::size_t>(-1);

    /*
    ** Закреплённый шаг пути /a/b[2]/c. ordinal — номер среди одноимённых
    ** соседей; numbered — ставить ли его в путь: только если одноимённых
    ** соседей несколько, а это становится известно лишь при закрытии
    ** родителя (в потоке последующие соседи ещё не пришли).
    **
    ** A pinned step of the path /a/b[2]/c. ordinal is the number among same-named
    ** siblings; numbered tells whether to put it into the path: only if there are several
    ** same-named siblings, and this becomes known only when the parent is
    ** closed (in a stream the following siblings have not arrived yet).
    */
    struct PathStep
    {
        std::size_t parent;
        std::string_view name;
        std::size_t ordinal;
        bool numbered;
    };

    // Счётчик детей с одним именем у открытого элемента
    // A counter of children with one name at an open element
    struct SiblingCount
    {
        std::string name;
        std::size_t count;
    };

    /*
    ** Уровень открытого элемента. Объекты уровней переиспользуются (память
    ** строк и счётчиков остаётся), поэтому открытие элемента после прогрева
    ** не выделяет память. Счётчики детей — небольшой массив, а у элемента
    ** с множеством разных имён детей — хеш-таблица с секретным ключом.
    **
    ** The level of an open element. Level objects are reused (the memory of
    ** strings and counters stays), so opening an element after warm-up
    ** does not allocate memory. Child counters are a small array, and for an element
    ** with many different child names — a hash table with a secret key.
    */
    struct PathLevel
    {
        std::string name;
        std::size_t ordinal = 0;
        std::size_t step = NO_STEP;
        std::vector<SiblingCount> few;
        std::size_t fewUsed = 0;
        std::size_t lastHit = 0;
        std::unordered_map<std::string_view, std::size_t, utils::SeededHash> many;
        std::vector<std::size_t> pinnedChildren;
    };

    /*
    ** PathTracker — пути к элементам для сообщений без дерева и без
    ** строки пути у каждого элемента. Для открытых элементов хранятся лишь
    ** номера среди соседей; строка нужна только при ошибке, и тогда цепочка
    ** открытых предков «закрепляется» (pin) в таблицу шагов. Текст пути
    ** собирается в самом конце, когда известны все номера.
    **
    ** PathTracker — paths to elements for messages without a tree and without
    ** a path string for every element. For open elements only the numbers among
    ** siblings are kept; a string is needed only on an error, and then the chain
    ** of open ancestors is "pinned" into the table of steps. The path text
    ** is assembled at the very end, when all the numbers are known.
    */
    class PathTracker
    {
    public:
        PathTracker();

        // Открытие элемента: его номер среди одноимённых соседей считается сразу
        // Opening an element: its number among same-named siblings is computed at once
        void open(std::string_view name);

        // Закрытие: у закреплённых детей становится известно, нужен ли номер
        // Closing: for the pinned children it becomes known whether a number is needed
        void close();

        // Шаг открытого элемента уровня level (0 — корень), закрепляемый при первом запросе
        // The step of the open element of the level level (0 — the root), pinned on the first request
        std::size_t pin(std::size_t level);

        // Текст пути шага: /a/b[2]/c
        // The path text of a step: /a/b[2]/c
        std::string text(std::size_t step) const;

    private:
        // ---- Счётчики соседей (path_tracker_counts.cpp) ----
        // ---- Sibling counters (path_tracker_counts.cpp) ----
        std::size_t countChild(PathLevel& parent, std::string_view name);
        std::size_t countMany(PathLevel& parent, std::string_view name);
        std::size_t countOf(const PathLevel& parent, std::string_view name) const;
        std::string_view intern(std::string_view name);

        std::vector<PathLevel> _levels;
        std::size_t _depth;
        std::vector<PathStep> _steps;
        std::deque<std::string> _names;
        std::unordered_map<std::string_view, std::string_view, utils::SeededHash> _nameIndex;
    };
}

#endif
