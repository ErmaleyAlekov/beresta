#include "content/content_matcher.hpp"

namespace beresta::content
{
    // Лист (элемент или xs:any): из позиции s можно перейти в s+1, если ребёнок подходит
    PositionSet ContentMatcher::matchLeaf(const Particle& particle, const PositionSet& starts)
    {
        PositionSet result = emptySet();

        for (std::size_t position = 0; position < _children.size(); ++position)
        {
            if (!starts[position] || !particleAccepts(particle, _children[position]))
                continue;
            result[position + 1] = 1;
            if (position + 1 > _furthest)
                _furthest = position + 1;
        }
        return result;
    }

    // Последовательность: позиции «протаскиваются» через каждую частицу по очереди
    PositionSet ContentMatcher::matchSequence(const Particle& particle, const PositionSet& starts)
    {
        PositionSet current = starts;

        for (const Particle& child : particle.children)
        {
            current = matchRepeated(child, current);
            if (isEmptySet(current))
                break;
        }
        return current;
    }

    // Выбор: объединение результатов всех вариантов
    PositionSet ContentMatcher::matchChoice(const Particle& particle, const PositionSet& starts)
    {
        PositionSet result = emptySet();

        for (const Particle& child : particle.children)
            unitePositions(result, matchRepeated(child, starts));
        return result;
    }

    // Объединение множеств с признаком «появилось новое»
    bool unitePositions(PositionSet& target, const PositionSet& source) noexcept
    {
        bool changed = false;

        for (std::size_t index = 0; index < target.size() && index < source.size(); ++index)
        {
            if (source[index] && !target[index])
            {
                target[index] = 1;
                changed = true;
            }
        }
        return changed;
    }

    // Пустота множества
    bool isEmptySet(const PositionSet& positions) noexcept
    {
        for (char flag : positions)
        {
            if (flag)
                return false;
        }
        return true;
    }
}
