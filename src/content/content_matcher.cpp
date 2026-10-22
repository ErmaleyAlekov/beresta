#include "content/content_matcher.hpp"

#include <algorithm>

namespace beresta::content
{
    // Сопоставитель привязан к списку детей одного элемента
    // The matcher is bound to the child list of one element
    ContentMatcher::ContentMatcher(const std::vector<ChildKey>& children)
        : _children(children), _furthest(0)
    {
    }

    /*
    ** Повторения частицы. На шаге i множество current — позиции после i
    ** применений. Начиная с minOccurs позиции попадают в результат.
    ** Дальше продолжаются только НОВЫЕ позиции: позиция, уже попавшая в
    ** результат раньше, была достигнута меньшим числом повторов, и её
    ** продолжения уже учтены (оставшийся запас повторов у неё не меньше).
    ** Цикл останавливается на maxOccurs, на пустом множестве или когда
    ** новых позиций нет — замыкание достигнуто. Так item* над n детьми
    ** стоит O(n), а «пустая» частица с unbounded не зацикливается.
    ** Частица ровно с одним вхождением (самый частый случай) — одно применение.
    **
    ** Particle repetitions. At step i the set current holds the positions after i
    ** applications. Starting from minOccurs the positions go into the result.
    ** Only NEW positions are continued: a position that got into the result earlier
    ** was reached with fewer repetitions, and its continuations are already counted
    ** (its remaining repetition budget is not smaller). The loop stops at maxOccurs,
    ** on an empty set or when there are no new positions — the closure is reached.
    ** So item* over n children costs O(n), and an "empty" particle with unbounded
    ** does not loop forever. A particle with exactly one occurrence (the most
    ** common case) is a single application.
    */
    PositionSet ContentMatcher::matchRepeated(const Particle& particle, const PositionSet& starts)
    {
        PositionSet result;
        std::unordered_set<std::size_t> members;
        PositionSet current = starts;
        std::size_t extra = _children.size() + 1;
        std::size_t limit = static_cast<std::size_t>(-1);

        // Сложение с насыщением: огромный minOccurs не должен переполнить предел
        // A saturating addition: a huge minOccurs must not overflow the limit
        if (particle.minOccurs <= limit - extra)
            limit = particle.minOccurs + extra;
        if (particle.minOccurs == 1 && particle.maxOccurs == 1)
            return matchOnce(particle, starts);
        for (std::size_t count = 0; count <= limit && !current.empty(); ++count)
        {
            if (count >= particle.minOccurs)
                current = addNew(result, members, current);
            if (current.empty() || (particle.maxOccurs != UNBOUNDED && count == particle.maxOccurs))
                break;
            current = matchOnce(particle, current);
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    // Самая дальняя достигнутая позиция
    // The farthest reached position
    std::size_t ContentMatcher::getFurthest() const noexcept
    {
        return _furthest;
    }

    // Выбор алгоритма по виду частицы
    // Chooses the algorithm by the particle kind
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

    // Новые позиции попадают и в результат, и в возвращаемое множество (порядок сохраняется)
    // New positions go both into the result and into the returned set (the order is kept)
    PositionSet ContentMatcher::addNew(PositionSet& result, std::unordered_set<std::size_t>& members,
        const PositionSet& positions)
    {
        PositionSet fresh;

        for (std::size_t position : positions)
        {
            if (!members.insert(position).second)
                continue;
            result.push_back(position);
            fresh.push_back(position);
        }
        return fresh;
    }
}
