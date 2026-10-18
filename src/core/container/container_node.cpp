#include "core/container_node.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Пустой контейнер без детей
    // An empty container without children
    ContainerNode::ContainerNode() noexcept
        : Node()
    {
    }

    /*
    ** Глубокое копирование всего поддерева (без рекурсии, см.
    ** cloneChildren), после чего копии привязываются к новому
    ** контейнеру. Оригинал и копия не делят ни одного узла.
    **
    ** Deep copying of the whole subtree (without recursion, see
    ** cloneChildren), after which the copies are bound to the new
    ** container. The original and the copy share no node.
    */
    ContainerNode::ContainerNode(const ContainerNode& other)
        : Node(other), _children(cloneChildren(other))
    {
        adoptChildren();
    }

    // Перемещение: забираем детей целиком и перепривязываем их к себе
    // Moving: take the children whole and rebind them to itself
    ContainerNode::ContainerNode(ContainerNode&& other) noexcept
        : Node(std::move(other)), _children(std::move(other._children))
    {
        other._children.clear();
        adoptChildren();
    }

    /*
    ** Копирующее присваивание по идиоме copy-and-swap:
    ** сначала клонируем (может бросить исключение — объект не изменится),
    ** затем дёшево меняем списки местами. Это даёт строгую гарантию
    ** безопасности исключений и корректно обрабатывает самоприсваивание.
    **
    ** Copy assignment by the copy-and-swap idiom:
    ** first clone (it may throw an exception — the object does not change),
    ** then cheaply swap the lists. This gives the strong exception safety
    ** guarantee and handles self-assignment correctly.
    */
    ContainerNode& ContainerNode::operator=(const ContainerNode& other)
    {
        ChildList copy = cloneChildren(other);

        Node::operator=(other);
        _children.swap(copy);
        adoptChildren();
        return *this;
    }

    /*
    ** Перемещающее присваивание. Если this лежит внутри поддерева other,
    ** то this стал бы владельцем собственного предка — это цикл, запрещаем.
    ** Старых детей уничтожаем последними (в конце функции, вместе с taken),
    ** т.к. среди них может оказаться сам other.
    **
    ** Move assignment. If this lies inside the subtree of other,
    ** this would become the owner of its own ancestor — it is a cycle, forbidden.
    ** The old children are destroyed last (at the end of the function, with taken),
    ** because other itself may be among them.
    */
    ContainerNode& ContainerNode::operator=(ContainerNode&& other)
    {
        if (this == &other)
            return *this;
        if (isDescendantOf(other))
            throw DomException(DomErrorCode::HIERARCHY_VIOLATION,
                i18n::format(i18n::Message::MOVE_ANCESTOR_INTO_DESCENDANT));
        ChildList taken = std::move(other._children);
        other._children.clear();
        Node::operator=(std::move(other));
        _children.swap(taken);
        adoptChildren();
        return *this;
    }
}
