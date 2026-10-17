#include "content/content_matcher.hpp"

#include <set>

namespace beresta::content
{
    // Все обязательные (minOccurs >= 1) частицы группы использованы
    static bool requiredUsed(const Particle& group, std::uint64_t used) noexcept
    {
        for (std::size_t index = 0; index < group.children.size(); ++index)
        {
            bool isUsed = (used >> index) & 1u;
            if (group.children[index].minOccurs > 0 && !isUsed)
                return false;
        }
        return true;
    }

    /*
    ** xs:all — каждый ребёнок группы не более одного раза в любом порядке.
    ** Обход по состояниям «позиция + маска использованных частиц» с
    ** запоминанием посещённых. Группа xs:all по стандарту содержит только
    ** элементы с maxOccurs <= 1, поэтому маски на 64 бита достаточно.
    */
    PositionSet ContentMatcher::matchAll(const Particle& particle, const PositionSet& starts)
    {
        PositionSet result = emptySet();
        std::set<AllState> seen;
        std::vector<AllState> queue;

        for (std::size_t position = 0; position < starts.size(); ++position)
        {
            if (starts[position])
                queue.emplace_back(position, 0);
        }
        while (!queue.empty())
        {
            AllState state = queue.back();
            queue.pop_back();
            if (!seen.insert(state).second)
                continue;
            if (requiredUsed(particle, state.second))
                result[state.first] = 1;
            expandAllState(particle, state, queue);
        }
        return result;
    }

    // Пробуем каждую ещё не использованную частицу из текущей позиции
    void ContentMatcher::expandAllState(const Particle& particle, const AllState& state,
        std::vector<AllState>& queue)
    {
        for (std::size_t index = 0; index < particle.children.size() && index < 64; ++index)
        {
            std::uint64_t bit = std::uint64_t(1) << index;
            if (state.second & bit)
                continue;
            PositionSet single = emptySet();
            single[state.first] = 1;
            PositionSet next = matchOnce(particle.children[index], single);
            for (std::size_t end = 0; end < next.size(); ++end)
            {
                if (next[end])
                    queue.emplace_back(end, state.second | bit);
            }
        }
    }
}
