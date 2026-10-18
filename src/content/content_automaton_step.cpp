#include "content/internal/automaton_step.hpp"

namespace beresta::content::detail
{
    // Шаг привязан к ребёнку, набору результатов и рабочему стеку курсора
    // The step is bound to the child, the result set and the working stack of the cursor
    AutomatonStep::AutomatonStep(const ChildKey& child, ConfigSet& output, MatchConfig& stack)
        : _child(child), _output(output), _stack(stack)
    {
    }

    // Первый ребёнок начинает первое вхождение корня модели (если вхождения вообще разрешены)
    // The first child starts the first occurrence of the model root (if occurrences are allowed at all)
    void AutomatonStep::fromStart(const Particle& model)
    {
        OccurrenceBounds bounds{model.minOccurs, model.maxOccurs};

        _stack.clear();
        if (bounds.maxOccurs != 0)
            descend(model, saturate(1, bounds));
    }

    // Конфигурация копируется в рабочий стек: лист в её вершине только что принял ребёнка
    // The configuration is copied into the working stack: the leaf on its top has just accepted a child
    void AutomatonStep::fromConfig(const MatchConfig& config)
    {
        _stack = config;
        climb();
    }

    // Ребёнок xs:all — ровно одно применение; остальные — собственные minOccurs / maxOccurs
    // A child of xs:all is exactly one application; the others have their own minOccurs / maxOccurs
    OccurrenceBounds boundsAt(const MatchConfig& config, std::size_t level) noexcept
    {
        const Particle& particle = *config[level].particle;

        if (level > 0 && config[level - 1].particle->kind == ParticleKind::ALL)
            return OccurrenceBounds{0, 1};
        return OccurrenceBounds{particle.minOccurs, particle.maxOccurs};
    }

    // Номера сверх minOccurs у неограниченной частицы неразличимы — сводятся к minOccurs
    // Numbers above minOccurs of an unbounded particle are indistinguishable — reduced to minOccurs
    std::size_t saturate(std::size_t count, const OccurrenceBounds& bounds) noexcept
    {
        if (bounds.maxOccurs == UNBOUNDED && count > bounds.minOccurs)
            return bounds.minOccurs;
        return count;
    }
}
