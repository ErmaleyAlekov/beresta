#include "content/content_cursor.hpp"
#include "content/internal/automaton_step.hpp"

namespace beresta::content
{
    /*
    ** Есть ли в модели «вложенный счётчик»: частица с minOccurs > 1 или с
    ** конечным maxOccurs > 1 внутри повторяемой группы. Только такие модели
    ** могут дать автомату много конфигураций сразу (произведение
    ** счётчиков, как в ((a{2,5}){3,7})): их проверяет общий алгоритм на
    ** множествах позиций, полиномиальный при любой модели.
    **
    ** Whether the model has a "nested counter": a particle with minOccurs > 1 or with
    ** a finite maxOccurs > 1 inside a repeated group. Only such models
    ** can give the automaton many configurations at once (a product of
    ** counters, as in ((a{2,5}){3,7})): they are checked by the general algorithm on
    ** position sets, which is polynomial for any model.
    */
    static bool hasNestedCounter(const Particle& particle, bool repeated) noexcept
    {
        bool counted = particle.minOccurs > 1 || (particle.maxOccurs > 1 && particle.maxOccurs != UNBOUNDED);
        bool inner = repeated || particle.maxOccurs > 1;

        if (repeated && counted)
            return true;
        for (const Particle& child : particle.children)
        {
            if (hasNestedCounter(child, inner))
                return true;
        }
        return false;
    }

    // Самый быстрый подходящий способ: жадный, затем автомат, в крайнем случае буфер
    // The fastest fitting plan: greedy, then the automaton, as a last resort the buffer
    CursorPlan choosePlan(const Particle& model)
    {
        if (isGreedySequence(model))
            return CursorPlan::GREEDY_SEQUENCE;
        if (hasNestedCounter(model, false))
            return CursorPlan::BUFFERED;
        return CursorPlan::AUTOMATON;
    }

    /*
    ** Пустое вхождение: у листа его не бывает; sequence пуста, если пропускаемы
    ** все дети; choice — если пропускаем хоть один вариант; all — если
    ** необязательны (или пусты) все частицы. Совпадает с тем, когда общий
    ** алгоритм оставляет позицию на месте после одного применения частицы.
    **
    ** An empty occurrence: a leaf never has one; a sequence is empty if all
    ** children are skippable; a choice — if at least one alternative is skippable; all — if
    ** all particles are optional (or empty). It matches the case when the general
    ** algorithm keeps the position in place after one application of the particle.
    */
    bool isNullableOnce(const Particle& particle) noexcept
    {
        if (particle.kind == ParticleKind::ELEMENT || particle.kind == ParticleKind::WILDCARD)
            return false;
        if (particle.kind == ParticleKind::ALL)
            return detail::allGroupDone(particle, 0);
        bool sequence = particle.kind == ParticleKind::SEQUENCE;
        for (const Particle& child : particle.children)
        {
            bool skippable = detail::isSkippable(child);
            if (sequence && !skippable)
                return false;
            if (!sequence && skippable)
                return true;
        }
        return sequence;
    }
}
