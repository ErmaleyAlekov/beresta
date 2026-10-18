#ifndef CONTENT_MATCHER_HPP
#define CONTENT_MATCHER_HPP

#include "content/particle.hpp"

#include <cstddef>
#include <cstdint>
#include <unordered_set>
#include <utility>
#include <vector>

namespace beresta::content
{
    /*
    ** Множество позиций в списке детей: позиции 0..n по возрастанию, без
    ** повторов. Хранятся только достижимые позиции, а не флаг для каждой
    ** из n+1: обычно их единицы, и работа шага пропорциональна им, а не
    ** числу детей. Иначе модель item* над n детьми стоила бы O(n²).
    **
    ** A set of positions in the child list: positions 0..n in ascending order,
    ** without duplicates. Only reachable positions are kept, not a flag for each
    ** of the n+1: there are usually a few of them, and the work of a step is
    ** proportional to them, not to the number of children. Otherwise the item*
    ** model over n children would cost O(n²).
    */
    using PositionSet = std::vector<std::size_t>;

    /*
    ** ContentMatcher — сопоставление детей с моделью через множества
    ** достижимых позиций (по сути, симуляция НКА без построения автомата).
    ** Для каждой частицы вычисляется, в каких позициях списка детей можно
    ** оказаться после её применения. Множество ограничено n+1 позициями,
    ** поэтому время полиномиально — никакого экспоненциального перебора.
    ** Попутно запоминается самая дальняя достигнутая позиция: она и есть
    ** место ошибки, о котором сообщается пользователю.
    **
    ** ContentMatcher — matching children against a model through sets of
    ** reachable positions (in essence, an NFA simulation without building the automaton).
    ** For each particle it computes at which positions of the child list one can
    ** end up after applying it. The set is limited to n+1 positions,
    ** so the time is polynomial — no exponential search.
    ** Along the way the farthest reached position is remembered: it is the
    ** error location reported to the user.
    */
    class ContentMatcher
    {
    public:
        explicit ContentMatcher(const std::vector<ChildKey>& children);

        // Позиции после применения частицы с учётом minOccurs / maxOccurs
        // Positions after applying a particle, taking minOccurs / maxOccurs into account
        PositionSet matchRepeated(const Particle& particle, const PositionSet& starts);

        // Самая дальняя позиция, до которой удалось дойти
        // The farthest position that was reached
        std::size_t getFurthest() const noexcept;

    private:
        // Одно применение частицы (без повторений)
        // One application of a particle (without repetitions)
        PositionSet matchOnce(const Particle& particle, const PositionSet& starts);
        PositionSet matchLeaf(const Particle& particle, const PositionSet& starts);
        PositionSet matchSequence(const Particle& particle, const PositionSet& starts);
        PositionSet matchChoice(const Particle& particle, const PositionSet& starts);
        PositionSet matchAll(const Particle& particle, const PositionSet& starts);

        // Состояние обхода xs:all: позиция в списке детей и маска использованных частиц
        // The xs:all traversal state: the position in the child list and the mask of used particles
        using AllState = std::pair<std::size_t, std::uint64_t>;

        // Добавляет в очередь все состояния, достижимые из state одной неиспользованной частицей
        // Queues all states reachable from state with one unused particle
        void expandAllState(const Particle& particle, const AllState& state,
            std::vector<AllState>& queue);

        // Добавляет в result позиции, которых ещё нет в members; возвращает только новые
        // Adds to result the positions that are not in members yet; returns only the new ones
        static PositionSet addNew(PositionSet& result, std::unordered_set<std::size_t>& members,
            const PositionSet& positions);

        const std::vector<ChildKey>& _children;
        std::size_t _furthest;
    };

    // Объединение двух упорядоченных множеств слиянием за O(a + b)
    // The union of two ordered sets by merging in O(a + b)
    PositionSet mergePositions(const PositionSet& first, const PositionSet& second);

    // Быстрый жадный проход для последовательности элементов с разными именами (content_fast_path.cpp)
    // A fast greedy pass for a sequence of elements with different names (content_fast_path.cpp)
    bool matchesSimpleSequence(const Particle& model, const std::vector<ChildKey>& children);
}

#endif
