#include "core/container_node.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

namespace beresta
{
    // Делаем this родителем каждого ребёнка (нужно после копирования и перемещения)
    // Makes this the parent of every child (needed after copying and moving)
    void ContainerNode::adoptChildren() noexcept
    {
        for (std::unique_ptr<Node>& child : _children)
            child->setParent(this);
        reindexFrom(0);
    }

    /*
    ** Общие правила вставки (часть Template Method):
    ** 1) узел должен существовать;
    ** 2) узел не должен уже принадлежать другому родителю;
    ** 3) нельзя вставить себя или своего предка — получится цикл
    **    (узел без детей не может быть предком: подъём по предкам за
    **    O(глубина) нужен только для непустого контейнера);
    ** 4) наследник проверяет допустимость типа через validateChild().
    **
    ** The common insertion rules (a part of the Template Method):
    ** 1) the node must exist;
    ** 2) the node must not already belong to another parent;
    ** 3) one cannot insert oneself or one's ancestor — that would be a cycle
    **    (a node without children cannot be an ancestor: climbing the ancestors in
    **    O(depth) is needed only for a non-empty container);
    ** 4) the subclass checks that the type is allowed through validateChild().
    */
    void ContainerNode::checkInsertion(const Node* child, std::size_t index) const
    {
        if (child == nullptr)
            throw DomException(DomErrorCode::NULL_NODE, i18n::format(i18n::Message::INSERT_NULL_NODE));
        if (child->getParent() != nullptr)
            throw DomException(DomErrorCode::HIERARCHY_VIOLATION,
                i18n::format(i18n::Message::NODE_HAS_PARENT));
        const ContainerNode* container = asContainer(*child);
        bool mayContainThis = container != nullptr && !container->_children.empty();
        if (child == this || (mayContainThis && isDescendantOf(*child)))
            throw DomException(DomErrorCode::HIERARCHY_VIOLATION,
                i18n::format(i18n::Message::INSERT_INTO_SELF));
        validateChild(*child, index);
    }
}
