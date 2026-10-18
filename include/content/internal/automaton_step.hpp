#ifndef CONTENT_AUTOMATON_STEP_HPP
#define CONTENT_AUTOMATON_STEP_HPP

#include "content/content_cursor.hpp"

#include <cstddef>

namespace beresta::content::detail
{
    // Границы числа вхождений частицы на своём уровне конфигурации
    // The bounds of the number of occurrences of a particle at its configuration level
    struct OccurrenceBounds
    {
        std::size_t minOccurs;
        std::size_t maxOccurs;
    };

    /*
    ** Границы уровня level. Ребёнок xs:all по правилам общего алгоритма
    ** (matchAll применяет каждую частицу ровно один раз) входит не более
    ** одного раза, а его обязательность проверяет сама группа.
    **
    ** The bounds of the level level. A child of xs:all, by the rules of the general algorithm
    ** (matchAll applies each particle exactly once), occurs at most
    ** once, and its being mandatory is checked by the group itself.
    */
    OccurrenceBounds boundsAt(const MatchConfig& config, std::size_t level) noexcept;

    /*
    ** Номер вхождения с «насыщением»: при maxOccurs="unbounded" все
    ** номера не меньше minOccurs ведут себя одинаково (повтор разрешён,
    ** завершение тоже), поэтому они сводятся к minOccurs. Без этого
    ** item* давал бы новую конфигурацию на каждого ребёнка.
    **
    ** An occurrence number with "saturation": with maxOccurs="unbounded" all
    ** numbers not less than minOccurs behave the same (a repetition is allowed,
    ** finishing too), so they are reduced to minOccurs. Without this
    ** item* would give a new configuration for every child.
    */
    std::size_t saturate(std::size_t count, const OccurrenceBounds& bounds) noexcept;

    // Можно ли пропустить частицу целиком (ноль вхождений или пустое вхождение)
    // Whether the particle can be skipped entirely (zero occurrences or an empty occurrence)
    bool isSkippable(const Particle& particle) noexcept;

    // Все обязательные частицы xs:all использованы (или могут быть пустыми)
    // All mandatory particles of xs:all are used (or may be empty)
    bool allGroupDone(const Particle& group, std::uint64_t used) noexcept;

    /*
    ** AutomatonStep — один шаг автомата: из конфигураций до ребёнка
    ** получить все конфигурации, в которых этот ребёнок принят листом.
    ** Переходы из конфигурации — «подъём» (завершение текущих вхождений
    ** снизу вверх) и «спуск» (начало новых вхождений сверху вниз до листа).
    ** Вхождение, начатое на этом шаге, обязано содержать нового ребёнка,
    ** поэтому завершать его без листа нельзя — так пустые повторения не
    ** зацикливают обход. Пустые вхождения учитываются иначе: частицу,
    ** способную быть пустой, разрешено пропустить или завершить раньше
    ** minOccurs (недостающие вхождения — пустые).
    **
    ** AutomatonStep — one automaton step: from the configurations before a child
    ** get all configurations in which this child is accepted by a leaf.
    ** The transitions from a configuration are "climbing" (finishing the current occurrences
    ** bottom-up) and "descending" (starting new occurrences top-down to a leaf).
    ** An occurrence started at this step must contain the new child,
    ** so it cannot be finished without a leaf — this way empty repetitions do not
    ** make the traversal loop forever. Empty occurrences are taken into account differently: a particle
    ** that can be empty may be skipped or finished before
    ** minOccurs (the missing occurrences are empty).
    */
    class AutomatonStep
    {
    public:
        AutomatonStep(const ChildKey& child, ConfigSet& output, MatchConfig& stack);

        // Первый ребёнок: новое вхождение корня модели
        // The first child: a new occurrence of the model root
        void fromStart(const Particle& model);

        // Следующий ребёнок: продолжения конфигурации config
        // The next child: the continuations of the configuration config
        void fromConfig(const MatchConfig& config);

    private:
        // ---- Подъём (content_automaton_climb.cpp) ----
        // ---- Climbing (content_automaton_climb.cpp) ----
        void climb();
        void continueGroup();
        void continueSequence();
        void continueAll();

        // ---- Спуск (content_automaton_descend.cpp) ----
        // ---- Descending (content_automaton_descend.cpp) ----
        void descend(const Particle& particle, std::size_t count);
        void enterChild(std::size_t index);
        void descendGroup();

        const ChildKey& _child;
        ConfigSet& _output;
        MatchConfig& _stack;
    };
}

#endif
