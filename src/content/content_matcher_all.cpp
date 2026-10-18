#include "content/content_matcher.hpp"

#include <algorithm>
#include <set>

namespace beresta::content
{
    // Все обязательные (minOccurs >= 1) частицы группы использованы
    // All mandatory (minOccurs >= 1) particles of the group are used
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
    **
    ** xs:all — each child of the group at most once in any order.
    ** A traversal over "position + mask of used particles" states with
    ** memoization of the visited ones. By the standard an xs:all group contains only
    ** elements with maxOccurs <= 1, so a 64-bit mask is enough.
    */
    PositionSet ContentMatcher::matchAll(const Particle& particle, const PositionSet& starts)
    {
        PositionSet result;
        std::set<AllState> seen;
        std::vector<AllState> queue;

        for (std::size_t position : starts)
            queue.emplace_back(position, 0);
        while (!queue.empty())
        {
            AllState state = queue.back();
            queue.pop_back();
            if (!seen.insert(state).second)
                continue;
            if (requiredUsed(particle, state.second))
                result.push_back(state.first);
            expandAllState(particle, state, queue);
        }
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

    // Пробуем каждую ещё не использованную частицу из текущей позиции
    // Try each not yet used particle from the current position
    void ContentMatcher::expandAllState(const Particle& particle, const AllState& state,
        std::vector<AllState>& queue)
    {
        for (std::size_t index = 0; index < particle.children.size() && index < 64; ++index)
        {
            std::uint64_t bit = std::uint64_t(1) << index;
            if (state.second & bit)
                continue;
            PositionSet next = matchOnce(particle.children[index], PositionSet{state.first});
            for (std::size_t end : next)
                queue.emplace_back(end, state.second | bit);
        }
    }
}
