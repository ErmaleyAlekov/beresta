#include "content/content_matcher.hpp"

namespace beresta::content
{
    // Сопоставитель привязан к списку детей одного элемента
    ContentMatcher::ContentMatcher(const std::vector<ChildKey>& children)
        : _children(children), _furthest(0)
    {
    }

    /*
    ** Повторения частицы. На шаге i множество cur — позиции после i
    ** применений. Начиная с minOccurs позиции попадают в результат.
    ** Цикл останавливается на maxOccurs, на пустом множестве или когда
    ** очередное применение не даёт позиций сверх уже накопленных: тогда
    ** замыкание достигнуто (иначе «пустая» частица с unbounded зациклилась бы).
    */
    PositionSet ContentMatcher::matchRepeated(const Particle& particle, const PositionSet& starts)
    {
        PositionSet result = emptySet();
        PositionSet current = starts;
        std::size_t limit = particle.minOccurs + _children.size() + 1;

        for (std::size_t count = 0; count <= limit; ++count)
        {
            if (count >= particle.minOccurs)
                unitePositions(result, current);
            if (particle.maxOccurs != UNBOUNDED && count == particle.maxOccurs)
                break;
            PositionSet next = matchOnce(particle, current);
            PositionSet probe = result;
            if (isEmptySet(next))
                break;
            if (count >= particle.minOccurs && !unitePositions(probe, next))
                break;
            current = next;
        }
        return result;
    }

    // Самая дальняя достигнутая позиция
    std::size_t ContentMatcher::getFurthest() const noexcept
    {
        return _furthest;
    }

    // Выбор алгоритма по виду частицы
    PositionSet ContentMatcher::matchOnce(const Particle& particle, const PositionSet& starts)
    {
        if (particle.kind == ParticleKind::SEQUENCE)
            return matchSequence(particle, starts);
        if (particle.kind == ParticleKind::CHOICE)
            return matchChoice(particle, starts);
        if (particle.kind == ParticleKind::ALL)
            return matchAll(particle, starts);
        return matchLeaf(particle, starts);
    }

    // Пустое множество позиций 0..n
    PositionSet ContentMatcher::emptySet() const
    {
        return PositionSet(_children.size() + 1, 0);
    }
}
