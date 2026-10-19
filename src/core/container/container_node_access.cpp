#include "core/container_node.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

namespace beresta
{
    // Число непосредственных детей
    // The number of direct children
    std::size_t ContainerNode::getChildCount() const noexcept
    {
        return _children.size();
    }

    // Изменяемый доступ к ребёнку: переиспользуем константную версию (DRY)
    // Mutable access to a child: reuses the const version (DRY)
    Node* ContainerNode::getChild(std::size_t index)
    {
        const ContainerNode& self = *this;

        return const_cast<Node*>(self.getChild(index));
    }

    // Константный доступ с проверкой границ
    // Const access with a bounds check
    const Node* ContainerNode::getChild(std::size_t index) const
    {
        if (index >= _children.size())
            throw DomException(DomErrorCode::INDEX_OUT_OF_RANGE,
                i18n::format(i18n::Message::NO_CHILD_AT_INDEX, {std::to_string(index)}));
        return _children[index].get();
    }

    /*
    ** Индекс ребёнка за O(1): узел сам хранит свой номер среди соседей
    ** (его же использует getNextSibling). Раньше здесь был перебор
    ** списка, и перенос N детей с конца (beresta_node_move_to из обёрток,
    ** removeChild) стоил O(N²). Номер сверяется со списком: чужой узел
    ** или узел без родителя дают npos, а не ложный индекс.
    **
    ** The index of a child in O(1): the node itself stores its number among
    ** its siblings (getNextSibling uses it too). There used to be a list scan
    ** here, and moving N children from the end (beresta_node_move_to from the
    ** wrappers, removeChild) cost O(N²). The number is checked against the list:
    ** a foreign node or a node without a parent gives npos, not a false index.
    */
    std::size_t ContainerNode::indexOf(const Node* child) const noexcept
    {
        if (child == nullptr || child->_parent != this)
            return npos;
        std::size_t index = child->_siblingIndex;

        if (index < _children.size() && _children[index].get() == child)
            return index;
        return npos;
    }

    // Собираем текст всего поддерева в один буфер
    // Collects the text of the whole subtree into one buffer
    std::string ContainerNode::getTextContent() const
    {
        std::string output;

        collectText(*this, output);
        return output;
    }
}
