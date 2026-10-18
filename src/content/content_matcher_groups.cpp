#include "content/content_matcher.hpp"

namespace beresta::content
{
    // Лист (элемент или xs:any): из позиции s можно перейти в s+1, если ребёнок подходит
    // A leaf (an element or xs:any): from position s one can move to s+1 if the child fits
    PositionSet ContentMatcher::matchLeaf(const Particle& particle, const PositionSet& starts)
    {
        PositionSet result;

        for (std::size_t position : starts)
        {
            if (position >= _children.size() || !particleAccepts(particle, _children[position]))
                continue;
            result.push_back(position + 1);
            if (position + 1 > _furthest)
                _furthest = position + 1;
        }
        return result;
    }

    // Последовательность: позиции «протаскиваются» через каждую частицу по очереди
    // A sequence: the positions are "pulled" through each particle in turn
    PositionSet ContentMatcher::matchSequence(const Particle& particle, const PositionSet& starts)
    {
        PositionSet current = starts;

        for (const Particle& child : particle.children)
        {
            current = matchRepeated(child, current);
            if (current.empty())
                break;
        }
        return current;
    }

    // Выбор: объединение результатов всех вариантов
    // A choice: the union of the results of all alternatives
    PositionSet ContentMatcher::matchChoice(const Particle& particle, const PositionSet& starts)
    {
        PositionSet result;

        for (const Particle& child : particle.children)
            result = mergePositions(result, matchRepeated(child, starts));
        return result;
    }

    // Слияние двух упорядоченных множеств без повторов / Merging two ordered sets without duplicates
    PositionSet mergePositions(const PositionSet& first, const PositionSet& second)
    {
        PositionSet result;
        std::size_t left = 0;
        std::size_t right = 0;

        result.reserve(first.size() + second.size());
        while (left < first.size() || right < second.size())
        {
            if (right >= second.size() || (left < first.size() && first[left] < second[right]))
                result.push_back(first[left++]);
            else if (left >= first.size() || second[right] < first[left])
                result.push_back(second[right++]);
            else
            {
                result.push_back(first[left++]);
                ++right;
            }
        }
        return result;
    }
}
