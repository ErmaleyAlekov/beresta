#include "core/node.hpp"
#include "core/container_node.hpp"

namespace beresta
{
    // Изменяемая версия переиспользует константную (DRY)
    // The mutable version reuses the const one (DRY)
    Node* Node::getNextSibling()
    {
        const Node& self = *this;

        return const_cast<Node*>(self.getNextSibling());
    }

    /*
    ** Правый сосед — ребёнок родителя со следующим индексом.
    ** Индекс хранится в самом узле, поэтому поиска по списку нет: O(1).
    **
    ** The right sibling is the parent's child with the next index.
    ** The index is stored in the node itself, so there is no search in the list: O(1).
    */
    const Node* Node::getNextSibling() const
    {
        if (_parent == nullptr)
            return nullptr;
        if (_siblingIndex + 1 >= _parent->getChildCount())
            return nullptr;
        return _parent->getChild(_siblingIndex + 1);
    }

    // Изменяемая версия переиспользует константную (DRY)
    // The mutable version reuses the const one (DRY)
    Node* Node::getPreviousSibling()
    {
        const Node& self = *this;

        return const_cast<Node*>(self.getPreviousSibling());
    }

    // Левый сосед — ребёнок родителя с предыдущим индексом
    // The left sibling is the parent's child with the previous index
    const Node* Node::getPreviousSibling() const
    {
        if (_parent == nullptr || _siblingIndex == 0)
            return nullptr;
        return _parent->getChild(_siblingIndex - 1);
    }

    // Индекс поддерживается контейнером при каждой вставке и удалении
    // The index is maintained by the container on every insertion and removal
    std::size_t Node::getIndexInParent() const noexcept
    {
        return _siblingIndex;
    }
}
