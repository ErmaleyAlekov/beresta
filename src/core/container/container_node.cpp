#include "core/container_node.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Пустой контейнер без детей
    ContainerNode::ContainerNode() noexcept
        : Node()
    {
    }

    /*
    ** Глубокое копирование: каждый ребёнок клонируется рекурсивно через
    ** виртуальный clone(), после чего копии привязываются к новому
    ** контейнеру. Оригинал и копия не делят ни одного узла.
    */
    ContainerNode::ContainerNode(const ContainerNode& other)
        : Node(other), _children(cloneChildren(other._children))
    {
        adoptChildren();
    }

    // Перемещение: забираем детей целиком и перепривязываем их к себе
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
    */
    ContainerNode& ContainerNode::operator=(const ContainerNode& other)
    {
        ChildList copy = cloneChildren(other._children);

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
    */
    ContainerNode& ContainerNode::operator=(ContainerNode&& other)
    {
        if (this == &other)
            return *this;
        if (isDescendantOf(other))
            throw DomException(DomErrorCode::HIERARCHY_VIOLATION,
                "нельзя переместить предка внутрь его собственного потомка");
        ChildList taken = std::move(other._children);
        other._children.clear();
        Node::operator=(std::move(other));
        _children.swap(taken);
        adoptChildren();
        return *this;
    }
}
