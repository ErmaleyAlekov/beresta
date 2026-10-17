#include "core/node.hpp"
#include "core/container_node.hpp"

namespace beresta
{
    // Изменяемая версия переиспользует константную (DRY)
    Node* Node::getNextSibling()
    {
        const Node& self = *this;

        return const_cast<Node*>(self.getNextSibling());
    }

    /*
    ** Правый сосед — ребёнок родителя со следующим индексом.
    ** Индекс хранится в самом узле, поэтому поиска по списку нет: O(1).
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
    Node* Node::getPreviousSibling()
    {
        const Node& self = *this;

        return const_cast<Node*>(self.getPreviousSibling());
    }

    // Левый сосед — ребёнок родителя с предыдущим индексом
    const Node* Node::getPreviousSibling() const
    {
        if (_parent == nullptr || _siblingIndex == 0)
            return nullptr;
        return _parent->getChild(_siblingIndex - 1);
    }

    // Индекс поддерживается контейнером при каждой вставке и удалении
    std::size_t Node::getIndexInParent() const noexcept
    {
        return _siblingIndex;
    }
}
