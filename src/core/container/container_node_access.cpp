#include "core/container_node.hpp"
#include "exception/dom_exception.hpp"

namespace beresta
{
    // Число непосредственных детей
    std::size_t ContainerNode::getChildCount() const noexcept
    {
        return _children.size();
    }

    // Изменяемый доступ к ребёнку: переиспользуем константную версию (DRY)
    Node* ContainerNode::getChild(std::size_t index)
    {
        const ContainerNode& self = *this;

        return const_cast<Node*>(self.getChild(index));
    }

    // Константный доступ с проверкой границ
    const Node* ContainerNode::getChild(std::size_t index) const
    {
        if (index >= _children.size())
            throw DomException(DomErrorCode::INDEX_OUT_OF_RANGE,
                "нет ребёнка с индексом " + std::to_string(index));
        return _children[index].get();
    }

    // Линейный поиск ребёнка по адресу
    std::size_t ContainerNode::indexOf(const Node* child) const noexcept
    {
        for (std::size_t index = 0; index < _children.size(); ++index)
        {
            if (_children[index].get() == child)
                return index;
        }
        return npos;
    }

    // Собираем текст всего поддерева в один буфер
    std::string ContainerNode::getTextContent() const
    {
        std::string output;

        collectText(*this, output);
        return output;
    }
}
