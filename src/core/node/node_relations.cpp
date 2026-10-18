#include "core/node.hpp"

namespace beresta
{
    // Виртуальный деструктор: гарантирует вызов деструкторов наследников
    // A virtual destructor: guarantees that the subclass destructors are called
    Node::~Node()
    {
    }

    // Родитель для изменяемого узла
    // The parent for a mutable node
    ContainerNode* Node::getParent() noexcept
    {
        return _parent;
    }

    // Родитель для константного узла (константность распространяется и на него)
    // The parent for a const node (the constness extends to it too)
    const ContainerNode* Node::getParent() const noexcept
    {
        return _parent;
    }

    // Установка родителя — вызывается только из ContainerNode (friend)
    // Sets the parent — called only from ContainerNode (friend)
    void Node::setParent(ContainerNode* parent) noexcept
    {
        _parent = parent;
    }

    // Индекс среди детей родителя — поддерживается ContainerNode при каждой вставке/удалении
    // The index among the parent's children — maintained by ContainerNode on every insertion/removal
    void Node::setSiblingIndex(std::size_t index) noexcept
    {
        _siblingIndex = index;
    }
}
