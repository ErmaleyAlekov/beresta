#ifndef CONTENT_MATCHER_HPP
#define CONTENT_MATCHER_HPP

#include "content/particle.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace beresta::content
{
    // Множество позиций в списке детей: флаг для каждой позиции 0..n
    using PositionSet = std::vector<char>;

    /*
    ** ContentMatcher — сопоставление детей с моделью через множества
    ** достижимых позиций (по сути, симуляция НКА без построения автомата).
    ** Для каждой частицы вычисляется, в каких позициях списка детей можно
    ** оказаться после её применения. Множество ограничено n+1 позициями,
    ** поэтому время полиномиально — никакого экспоненциального перебора.
    ** Попутно запоминается самая дальняя достигнутая позиция: она и есть
    ** место ошибки, о котором сообщается пользователю.
    */
    class ContentMatcher
    {
    public:
        explicit ContentMatcher(const std::vector<ChildKey>& children);

        // Позиции после применения частицы с учётом minOccurs / maxOccurs
        PositionSet matchRepeated(const Particle& particle, const PositionSet& starts);

        // Самая дальняя позиция, до которой удалось дойти
        std::size_t getFurthest() const noexcept;

    private:
        // Одно применение частицы (без повторений)
        PositionSet matchOnce(const Particle& particle, const PositionSet& starts);
        PositionSet matchLeaf(const Particle& particle, const PositionSet& starts);
        PositionSet matchSequence(const Particle& particle, const PositionSet& starts);
        PositionSet matchChoice(const Particle& particle, const PositionSet& starts);
        PositionSet matchAll(const Particle& particle, const PositionSet& starts);

        // Состояние обхода xs:all: позиция в списке детей и маска использованных частиц
        using AllState = std::pair<std::size_t, std::uint64_t>;

        // Добавляет в очередь все состояния, достижимые из state одной неиспользованной частицей
        void expandAllState(const Particle& particle, const AllState& state,
            std::vector<AllState>& queue);

        // Пустое множество нужного размера
        PositionSet emptySet() const;

        const std::vector<ChildKey>& _children;
        std::size_t _furthest;
    };

    // Объединяет source в target; возвращает true, если добавилось что-то новое
    bool unitePositions(PositionSet& target, const PositionSet& source) noexcept;

    // Пусто ли множество
    bool isEmptySet(const PositionSet& positions) noexcept;
}

#endif
