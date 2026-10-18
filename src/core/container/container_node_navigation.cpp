#include "core/container_node.hpp"

namespace beresta
{
    // Изменяемая версия переиспользует константную (DRY)
    // The mutable version reuses the const one (DRY)
    Node* ContainerNode::getFirstChild()
    {
        const ContainerNode& self = *this;

        return const_cast<Node*>(self.getFirstChild());
    }

    // Первый ребёнок или nullptr для пустого контейнера
    // The first child or nullptr for an empty container
    const Node* ContainerNode::getFirstChild() const
    {
        if (_children.empty())
            return nullptr;
        return _children.front().get();
    }

    // Изменяемая версия переиспользует константную (DRY)
    // The mutable version reuses the const one (DRY)
    Node* ContainerNode::getLastChild()
    {
        const ContainerNode& self = *this;

        return const_cast<Node*>(self.getLastChild());
    }

    // Последний ребёнок или nullptr для пустого контейнера
    // The last child or nullptr for an empty container
    const Node* ContainerNode::getLastChild() const
    {
        if (_children.empty())
            return nullptr;
        return _children.back().get();
    }

    // Индекс первого ребёнка нужного типа (например, корня в документе)
    // The index of the first child of the required type (for example, the root in a document)
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
