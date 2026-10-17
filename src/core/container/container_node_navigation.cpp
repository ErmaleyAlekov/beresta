#include "core/container_node.hpp"

namespace beresta
{
    // Изменяемая версия переиспользует константную (DRY)
    Node* ContainerNode::getFirstChild()
    {
        const ContainerNode& self = *this;

        return const_cast<Node*>(self.getFirstChild());
    }

    // Первый ребёнок или nullptr для пустого контейнера
    const Node* ContainerNode::getFirstChild() const
    {
        if (_children.empty())
            return nullptr;
        return _children.front().get();
    }

    // Изменяемая версия переиспользует константную (DRY)
    Node* ContainerNode::getLastChild()
    {
        const ContainerNode& self = *this;

        return const_cast<Node*>(self.getLastChild());
    }

    // Последний ребёнок или nullptr для пустого контейнера
    const Node* ContainerNode::getLastChild() const
    {
        if (_children.empty())
            return nullptr;
        return _children.back().get();
    }

    // Индекс первого ребёнка нужного типа (например, корня в документе)
    std::size_t ContainerNode::findFirstOfType(NodeType type) const noexcept
    {
        for (std::size_t index = 0; index < _children.size(); ++index)
        {
            if (_children[index]->getType() == type)
                return index;
        }
        return npos;
    }
}
