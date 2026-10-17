#include "core/node.hpp"

namespace beresta
{
    // Виртуальный деструктор: гарантирует вызов деструкторов наследников
    Node::~Node()
    {
    }

    // Родитель для изменяемого узла
    ContainerNode* Node::getParent() noexcept
    {
        return _parent;
    }

    // Родитель для константного узла (константность распространяется и на него)
    const ContainerNode* Node::getParent() const noexcept
    {
        return _parent;
    }

    // Установка родителя — вызывается только из ContainerNode (friend)
    void Node::setParent(ContainerNode* parent) noexcept
    {
        _parent = parent;
    }

    // Индекс среди детей родителя — поддерживается ContainerNode при каждой вставке/удалении
    void Node::setSiblingIndex(std::size_t index) noexcept
    {
        _siblingIndex = index;
    }
}
