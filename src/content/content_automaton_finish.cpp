#include "content/internal/automaton_step.hpp"

#include <functional>

namespace beresta::content
{
    // Уровни равны, если это одна частица в одном вхождении с одинаковым продвижением
    // Levels are equal if this is one particle in one occurrence with the same progress
    bool operator==(const MatchFrame& left, const MatchFrame& right) noexcept
    {
        return left.particle == right.particle && left.count == right.count && left.progress == right.progress;
    }

    // Порядок уровней — любой строгий; нужен только упорядоченному множеству конфигураций
    // The order of levels is any strict one; it is needed only by the ordered set of configurations
    bool operator<(const MatchFrame& left, const MatchFrame& right) noexcept
    {
        if (left.particle != right.particle)
            return std::less<const Particle*>()(left.particle, right.particle);
        if (left.count != right.count)
            return left.count < right.count;
        return left.progress < right.progress;
    }

    // Может ли группа завершить текущее вхождение после своего текущего ребёнка
    // Whether the group can finish the current occurrence after its current child
    static bool groupCanEnd(const MatchFrame& frame) noexcept
    {
        const Particle& group = *frame.particle;

        if (group.kind == ParticleKind::ALL)
            return detail::allGroupDone(group, frame.progress);
        if (group.kind != ParticleKind::SEQUENCE)
            return true;
        for (std::size_t index = static_cast<std::size_t>(frame.progress) + 1; index < group.children.size(); ++index)
        {
            if (!detail::isSkippable(group.children[index]))
                return false;
        }
        return true;
    }

    /*
    ** Конец детей: каждый уровень снизу вверх обязан суметь завершиться,
    ** ничего больше не принимая, — вхождений хватает (или недостающие
    ** пусты), а оставшиеся части группы пропускаемы. Пустая конфигурация
    ** (детей не было) допустима, если модель может быть пустой.
    **
    ** The end of the children: every level bottom-up must be able to finish
    ** without accepting anything more — there are enough occurrences (or the missing ones
    ** are empty), and the remaining parts of the group are skippable. An empty configuration
    ** (there were no children) is allowed if the model can be empty.
    */
    bool canFinishConfig(const Particle& model, const MatchConfig& config) noexcept
    {
        if (config.empty())
            return model.minOccurs == 0 || isNullableOnce(model);
        for (std::size_t level = config.size(); level-- > 0;)
        {
            const MatchFrame& frame = config[level];
            detail::OccurrenceBounds bounds = detail::boundsAt(config, level);
            if (frame.count < bounds.minOccurs && !isNullableOnce(*frame.particle))
                return false;
            if (level > 0 && !groupCanEnd(config[level - 1]))
                return false;
        }
        return true;
    }
}
