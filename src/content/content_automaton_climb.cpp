#include "content/internal/automaton_step.hpp"

namespace beresta::content::detail
{
    /*
    ** Вершина стека только что завершила вхождение (лист принял ребёнка
    ** или группа дошла до конца). Два пути: повторить частицу (если
    ** maxOccurs позволяет) — спуском в новое вхождение, или завершить её
    ** (если вхождений хватает либо недостающие могут быть пустыми) — и
    ** продолжить родительскую группу. Корень завершать некуда: конец
    ** модели ребёнка не принимает. Стек после вызова прежний.
    **
    ** The top of the stack has just finished an occurrence (a leaf accepted a child
    ** or a group reached its end). Two ways: repeat the particle (if
    ** maxOccurs allows) — by descending into a new occurrence, or finish it
    ** (if there are enough occurrences or the missing ones may be empty) — and
    ** continue the parent group. The root cannot be finished into anything: the end
    ** of the model does not accept a child. The stack is the same after the call.
    */
    void AutomatonStep::climb()
    {
        std::size_t level = _stack.size() - 1;
        MatchFrame frame = _stack[level];
        OccurrenceBounds bounds = boundsAt(_stack, level);

        _stack.pop_back();
        if (bounds.maxOccurs == UNBOUNDED || frame.count < bounds.maxOccurs)
            descend(*frame.particle, saturate(frame.count + 1, bounds));
        if ((frame.count >= bounds.minOccurs || isNullableOnce(*frame.particle)) && !_stack.empty())
            continueGroup();
        _stack.push_back(frame);
    }

    // Текущий ребёнок группы в вершине стека завершён: что дальше — по виду группы
    // The current child of the group on the top of the stack is finished: what next depends on the group kind
    void AutomatonStep::continueGroup()
    {
        ParticleKind kind = _stack.back().particle->kind;

        if (kind == ParticleKind::SEQUENCE)
            continueSequence();
        else if (kind == ParticleKind::ALL)
            continueAll();
        else
            climb();
    }

    /*
    ** Последовательность: следующий ребёнок может начаться сразу, а
    ** пропускаемые (minOccurs="0" или пустые) можно перешагнуть. Если
    ** перешагнуть удалось все оставшиеся — вхождение последовательности
    ** завершено, и подъём продолжается выше.
    **
    ** A sequence: the next child may start right away, and the
    ** skippable ones (minOccurs="0" or empty ones) can be stepped over. If
    ** all the remaining ones could be stepped over — the occurrence of the sequence
    ** is finished, and the climbing continues higher.
    */
    void AutomatonStep::continueSequence()
    {
        std::size_t level = _stack.size() - 1;
        const Particle& group = *_stack[level].particle;
        std::uint64_t saved = _stack[level].progress;

        for (std::size_t index = static_cast<std::size_t>(saved) + 1; index < group.children.size(); ++index)
        {
            _stack[level].progress = index;
            enterChild(index);
            if (!isSkippable(group.children[index]))
            {
                _stack[level].progress = saved;
                return;
            }
        }
        _stack[level].progress = saved;
        climb();
    }

    /*
    ** xs:all: можно начать любую ещё не использованную частицу или, если
    ** все обязательные уже были, завершить вхождение группы. Маска на 64
    ** бита — как у общего алгоритма (matchAll).
    **
    ** xs:all: any not yet used particle may start or, if
    ** all the mandatory ones have already been, the occurrence of the group may finish. The 64-bit
    ** mask is the same as in the general algorithm (matchAll).
    */
    void AutomatonStep::continueAll()
    {
        std::size_t level = _stack.size() - 1;
        const Particle& group = *_stack[level].particle;
        std::uint64_t used = _stack[level].progress;

        for (std::size_t index = 0; index < group.children.size() && index < 64; ++index)
        {
            std::uint64_t bit = std::uint64_t(1) << index;
            if ((used & bit) != 0)
                continue;
            _stack[level].progress = used | bit;
            enterChild(index);
        }
        _stack[level].progress = used;
        if (allGroupDone(group, used))
            climb();
    }
}
