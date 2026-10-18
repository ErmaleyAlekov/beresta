#ifndef CONTENT_CURSOR_HPP
#define CONTENT_CURSOR_HPP

#include "content/particle.hpp"

#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace beresta::content
{
    /*
    ** Способ пошаговой проверки модели. Выбирается один раз для модели
    ** (choosePlan) по её форме — от самого быстрого к самому общему.
    **
    ** The way of step-by-step model checking. It is chosen once per model
    ** (choosePlan) by its shape — from the fastest to the most general.
    */
    enum class CursorPlan
    {
        GREEDY_SEQUENCE,    // последовательность элементов с разными именами: два счётчика
                            // a sequence of elements with different names: two counters
        AUTOMATON,          // общий случай: множество конфигураций частиц (НКА)
                            // the general case: a set of particle configurations (an NFA)
        BUFFERED            // вложенные ограниченные счётчики: дети копятся, проверка в конце
                            // nested bounded counters: the children are collected, checked at the end
    };

    /*
    ** Уровень конфигурации автомата: частица, номер её текущего вхождения
    ** и продвижение внутри него (для sequence — индекс текущего ребёнка,
    ** для choice — выбранный вариант, для all — маска использованных частиц).
    **
    ** A level of an automaton configuration: a particle, the number of its current
    ** occurrence and the progress inside it (for sequence — the index of the current child,
    ** for choice — the chosen alternative, for all — the mask of used particles).
    */
    struct MatchFrame
    {
        const Particle* particle;
        std::size_t count;
        std::uint64_t progress;
    };

    /*
    ** Конфигурация — путь от корня модели к листу, который принял
    ** последнего ребёнка. Пустая конфигурация — «ещё ничего не принято».
    **
    ** A configuration is the path from the model root to the leaf that accepted
    ** the last child. An empty configuration means "nothing has been accepted yet".
    */
    using MatchConfig = std::vector<MatchFrame>;

    // Равенство и порядок уровней — для удаления повторных конфигураций
    // Equality and order of levels — for removing repeated configurations
    bool operator==(const MatchFrame& left, const MatchFrame& right) noexcept;
    bool operator<(const MatchFrame& left, const MatchFrame& right) noexcept;

    /*
    ** Набор конфигураций без повторов. Объекты конфигураций не удаляются
    ** при очистке, а переиспользуются (count — число занятых), поэтому
    ** шаг автомата после прогрева не выделяет память. Повторы ищутся
    ** перебором, а для больших наборов — через упорядоченное множество.
    **
    ** A set of configurations without repetitions. Configuration objects are not deleted
    ** on clearing but reused (count is the number of occupied ones), so an
    ** automaton step does not allocate memory after warm-up. Repetitions are found
    ** by a loop, and for large sets — through an ordered set.
    */
    struct ConfigSet
    {
        std::vector<MatchConfig> items;
        std::size_t count = 0;
        std::set<MatchConfig> seen;

        void clear();
        void add(const MatchConfig& config);
    };

    // Ребёнок, сохранённый для отложенной проверки (свои строки вместо представлений)
    // A child saved for a deferred check (own strings instead of views)
    struct OwnedChildKey
    {
        std::string qualifiedName;
        std::string localName;
        std::string namespaceUri;
    };

    /*
    ** ContentCursor — пошаговая проверка детей одного элемента по модели
    ** (паттерн Iterator наоборот: не мы обходим детей, а дети приходят по
    ** одному из потока SAX). step() вызывается для каждого дочернего
    ** элемента, finish() — при закрытии родителя. Дерево и список детей не
    ** нужны: память — O(размера модели), а не O(числа детей).
    **
    ** Место ошибки совпадает с тем, что даёт matchContent над всем списком:
    ** первый ребёнок, после которого не осталось ни одного пути по модели.
    ** Объект переиспользуется (reset) для следующего элемента.
    **
    ** ContentCursor — step-by-step checking of the children of one element against a model
    ** (the Iterator pattern inside out: we do not walk the children, the children arrive
    ** one by one from the SAX stream). step() is called for each child
    ** element, finish() — when the parent is closed. Neither the tree nor the child list is
    ** needed: the memory is O(model size), not O(number of children).
    ** The error location is the same as matchContent gives over the whole list:
    ** the first child after which no path through the model remains.
    ** The object is reused (reset) for the next element.
    */
    class ContentCursor
    {
    public:
        ContentCursor();

        // Начало проверки нового элемента по модели model способом plan
        // Starts checking a new element against the model model with the plan plan
        void reset(const Particle& model, CursorPlan plan);

        // Очередной ребёнок; false — ребёнок недопустим (дальнейшие шаги ничего не делают)
        // The next child; false — the child is not allowed (further steps do nothing)
        bool step(const ChildKey& child);

        // Итог после последнего ребёнка: успех или индекс ошибки (число детей — «не хватает»)
        // The result after the last child: success or the error index (the number of children — "missing")
        MatchResult finish();

    private:
        // ---- Жадный путь (content_fast_path.cpp) ----
        // ---- The greedy path (content_fast_path.cpp) ----
        bool stepGreedy(const ChildKey& child);
        bool finishGreedy() const;

        // ---- Автомат и буфер (content_cursor_automaton.cpp) ----
        // ---- The automaton and the buffer (content_cursor_automaton.cpp) ----
        bool stepAutomaton(const ChildKey& child);
        bool finishAutomaton() const;
        void stepBuffered(const ChildKey& child);
        MatchResult finishBuffered();

        const Particle* _model;
        CursorPlan _plan;
        bool _failed;
        std::size_t _consumed;
        std::size_t _leaf;
        std::size_t _taken;
        ConfigSet _current;
        ConfigSet _next;
        MatchConfig _work;
        std::vector<OwnedChildKey> _buffered;
        std::size_t _bufferedCount;
    };

    // Способ проверки, подходящий модели (content_plan.cpp)
    // The checking plan that fits the model (content_plan.cpp)
    CursorPlan choosePlan(const Particle& model);

    // Может ли ОДНО вхождение частицы не содержать ни одного элемента (content_plan.cpp)
    // Whether ONE occurrence of the particle may contain no element at all (content_plan.cpp)
    bool isNullableOnce(const Particle& particle) noexcept;

    // Последовательность ровно один раз из элементов с попарно разными именами (content_fast_path.cpp)
    // A sequence exactly once of elements with pairwise different names (content_fast_path.cpp)
    bool isGreedySequence(const Particle& model) noexcept;

    // Можно ли закончить модель в конфигурации config (content_automaton_finish.cpp)
    // Whether the model can be finished in the configuration config (content_automaton_finish.cpp)
    bool canFinishConfig(const Particle& model, const MatchConfig& config) noexcept;
}

#endif
