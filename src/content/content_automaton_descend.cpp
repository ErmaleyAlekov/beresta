#include "content/internal/automaton_step.hpp"

namespace beresta::content::detail
{
    /*
    ** Новое вхождение частицы с номером count. Лист принимает ребёнка —
    ** и тогда стек целиком становится новой конфигурацией; группа ищет
    ** лист среди своих детей. Стек после вызова прежний.
    **
    ** A new occurrence of the particle with the number count. A leaf accepts the child —
    ** and then the whole stack becomes a new configuration; a group looks for
    ** a leaf among its children. The stack is the same after the call.
    */
    void AutomatonStep::descend(const Particle& particle, std::size_t count)
    {
        _stack.push_back(MatchFrame{&particle, count, 0});
        if (particle.kind == ParticleKind::ELEMENT || particle.kind == ParticleKind::WILDCARD)
        {
            if (particleAccepts(particle, _child))
                _output.add(_stack);
        }
        else
            descendGroup();
        _stack.pop_back();
    }

    // Первое вхождение ребёнка index группы в вершине стека (если оно вообще разрешено)
    // The first occurrence of the child index of the group on the top of the stack (if it is allowed at all)
    void AutomatonStep::enterChild(std::size_t index)
    {
        const Particle& group = *_stack.back().particle;
        const Particle& child = group.children[index];
        OccurrenceBounds bounds{child.minOccurs, child.maxOccurs};

        if (group.kind == ParticleKind::ALL)
            bounds = OccurrenceBounds{0, 1};
        if (bounds.maxOccurs != 0)
            descend(child, saturate(1, bounds));
    }

    /*
    ** Начало вхождения группы. Sequence начинается с первого ребёнка,
    ** перешагивая пропускаемых; choice — с любого варианта; all — с любой
    ** частицы. Дойти до конца группы, ничего не приняв, нельзя: такое
    ** вхождение пустое, а пустые учитываются при подъёме.
    **
    ** The start of a group occurrence. A sequence starts with the first child,
    ** stepping over the skippable ones; a choice — with any alternative; all — with any
    ** particle. Reaching the end of the group without accepting anything is not allowed: such
    ** an occurrence is empty, and empty ones are taken into account while climbing.
    */
    void AutomatonStep::descendGroup()
    {
        std::size_t level = _stack.size() - 1;
        const Particle& group = *_stack[level].particle;

        for (std::size_t index = 0; index < group.children.size(); ++index)
        {
            if (group.kind == ParticleKind::ALL && index >= 64)
                break;
            _stack[level].progress = index;
            if (group.kind == ParticleKind::ALL)
                _stack[level].progress = std::uint64_t(1) << index;
            enterChild(index);
            if (group.kind == ParticleKind::SEQUENCE && !isSkippable(group.children[index]))
                break;
        }
    }

    // Пропустить можно частицу с minOccurs="0" или такую, чьё вхождение бывает пустым
    // A particle with minOccurs="0" or one whose occurrence can be empty can be skipped
    bool isSkippable(const Particle& particle) noexcept
    {
        return particle.minOccurs == 0 || isNullableOnce(particle);
    }

    /*
    ** Группа xs:all может завершиться, если каждая неиспользованная частица
    ** необязательна или может войти пустой (как в matchAll: такая частица
    ** «используется», не сдвигая позицию). Частицы дальше 64-й в маску не
    ** помещаются и потому использованы быть не могут.
    **
    ** An xs:all group can finish if every unused particle
    ** is optional or can occur empty (as in matchAll: such a particle
    ** is "used" without moving the position). Particles past the 64th do not fit
    ** into the mask and therefore cannot be used.
    */
    bool allGroupDone(const Particle& group, std::uint64_t used) noexcept
    {
        for (std::size_t index = 0; index < group.children.size(); ++index)
        {
            const Particle& child = group.children[index];
            bool isUsed = index < 64 && ((used >> index) & 1u) != 0;
            if (isUsed || child.minOccurs == 0)
                continue;
            if (index >= 64 || !isNullableOnce(child))
                return false;
        }
        return true;
    }
}
