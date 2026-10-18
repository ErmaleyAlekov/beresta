#include "xsd/internal/tree_identity.hpp"
#include "xsd/internal/identity_common.hpp"

#include <functional>
#include <utility>

namespace beresta::xsd::detail
{
    // Виртуальный деструктор интерфейса стратегии ключей
    // The virtual destructor of the key strategy interface
    IdentityTracker::~IdentityTracker()
    {
    }

    // Ячейки равны, если совпадают и элемент, и номер атрибута
    // Slots are equal if both the element and the attribute number match
    bool operator==(const AttributeSlot& left, const AttributeSlot& right) noexcept
    {
        return left.ordinal == right.ordinal && left.index == right.index;
    }

    // Смешивание двух номеров (константа из золотого сечения разносит соседние номера)
    // Mixing two numbers (the golden ratio constant spreads neighbouring numbers apart)
    std::size_t AttributeSlotHash::operator()(const AttributeSlot& slot) const noexcept
    {
        std::size_t seed = std::hash<std::size_t>()(slot.ordinal);

        return seed ^ (std::hash<std::size_t>()(slot.index) + 0x9e3779b9u + (seed << 6) + (seed >> 2));
    }

    /*
    ** Ошибка у элемента, найденного через XPath. Обход дерева сообщал о
    ** ключах после всех остальных ошибок и в порядке своей работы — номер
    ** _sequence сохраняет этот порядок. Путь — из записанного шага; у
    ** непроверявшегося элемента (например, внутри xs:any skip) — по дереву.
    ** Позиция — записанная в потоке (дерево могло строиться без позиций).
    **
    ** An error at an element found through XPath. The tree traversal reported
    ** keys after all other errors and in the order of its work — the number
    ** _sequence keeps this order. The path is from the recorded step; for
    ** an element that was not validated (for example, inside xs:any skip) — from the tree.
    ** The position is the one recorded in the stream (the tree may have been built without positions).
    */
    void TreeIdentityCheck::reportElement(const Element& element, std::string message)
    {
        DiagnosticOrder order{static_cast<std::size_t>(OrderGroup::KEYS), 0, 0, 0, _sequence++};
        auto found = _elementPaths.find(&element);
        auto recorded = _elementLocations.find(&element);
        SourceLocation location = element.getSourceLocation();

        if (recorded != _elementLocations.end())
            location = recorded->second;
        if (found != _elementPaths.end())
            _log.add(order, found->second, std::move(message), EventPosition{Locator::NO_MARK, location});
        else
            _log.addAtPath(order, pathOf(element), std::move(message), EventPosition{Locator::NO_MARK, location});
    }

    // Ошибка у самой области (селектор вернул не множество узлов)
    // An error at the scope itself (the selector returned not a node set)
    void TreeIdentityCheck::reportScope(const IdentityScope& scope, std::string message)
    {
        DiagnosticOrder order{static_cast<std::size_t>(OrderGroup::KEYS), 0, 0, 0, _sequence++};

        _log.add(order, scope.path, std::move(message), EventPosition{Locator::NO_MARK, scope.location});
    }
}
