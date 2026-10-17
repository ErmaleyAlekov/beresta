#include "core/container_node.hpp"
#include "core/character_node.hpp"
#include "exception/dom_exception.hpp"

namespace beresta
{
    // Детей сериализуем подряд: никаких отступов и переводов строк между ними
    void ContainerNode::serializeChildren(std::string& output) const
    {
        for (const std::unique_ptr<Node>& child : _children)
            child->serialize(output);
    }

    // Каждый ребёнок клонируется через виртуальный clone() — сохраняется реальный тип
    ContainerNode::ChildList ContainerNode::cloneChildren(const ChildList& source)
    {
        ChildList copies;

        copies.reserve(source.size());
        for (const std::unique_ptr<Node>& child : source)
            copies.push_back(child->clone());
        return copies;
    }

    /*
    ** Сбор текста: TEXT и CDATA дают свои значения, вложенные контейнеры
    ** обходятся рекурсивно, а комментарии и PI пропускаются (как в W3C DOM).
    */
    void ContainerNode::collectText(const ContainerNode& container, std::string& output)
    {
        for (const std::unique_ptr<Node>& child : container._children)
        {
            NodeType type = child->getType();
            if (type == NodeType::TEXT || type == NodeType::CDATA_SECTION)
                output += static_cast<const CharacterNode&>(*child).getValue();
            else if (type == NodeType::ELEMENT || type == NodeType::DOCUMENT)
                collectText(static_cast<const ContainerNode&>(*child), output);
        }
    }

    // Делаем this родителем каждого ребёнка (нужно после копирования и перемещения)
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
    ** 3) нельзя вставить себя или своего предка — получится цикл;
    ** 4) наследник проверяет допустимость типа через validateChild().
    */
    void ContainerNode::checkInsertion(const Node* child, std::size_t index) const
    {
        if (child == nullptr)
            throw DomException(DomErrorCode::NULL_NODE, "попытка вставить пустой узел");
        if (child->getParent() != nullptr)
            throw DomException(DomErrorCode::HIERARCHY_VIOLATION,
                "узел уже принадлежит другому родителю; сначала вызовите removeChild()");
        if (child == this || isDescendantOf(*child))
            throw DomException(DomErrorCode::HIERARCHY_VIOLATION,
                "нельзя вставить узел внутрь самого себя или своего потомка");
        validateChild(*child, index);
    }
}
