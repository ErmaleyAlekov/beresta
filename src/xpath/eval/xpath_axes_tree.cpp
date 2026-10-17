#include "xpath/internal/xpath_axes.hpp"
#include "core/container_node.hpp"

namespace beresta::xpath::detail
{
    // Дети есть только у элементов и документа
    const Node* firstChildNode(const Node* node)
    {
        if (node == nullptr)
            return nullptr;
        if (node->getType() != NodeType::ELEMENT && node->getType() != NodeType::DOCUMENT)
            return nullptr;
        return static_cast<const ContainerNode*>(node)->getFirstChild();
    }

    // Последний ребёнок контейнера
    const Node* lastChildNode(const Node* node)
    {
        if (node == nullptr)
            return nullptr;
        if (node->getType() != NodeType::ELEMENT && node->getType() != NodeType::DOCUMENT)
            return nullptr;
        return static_cast<const ContainerNode*>(node)->getLastChild();
    }

    /*
    ** Порядок документа без рекурсии: сначала вглубь (первый ребёнок),
    ** затем вправо, а при отсутствии соседа — вверх, но не выше root.
    */
    const Node* nextInDocument(const Node* node, const Node* root)
    {
        const Node* child = firstChildNode(node);

        if (child != nullptr)
            return child;
        for (const Node* cursor = node; cursor != nullptr && cursor != root;
            cursor = cursor->getParent())
        {
            if (cursor->getNextSibling() != nullptr)
                return cursor->getNextSibling();
        }
        return nullptr;
    }

    // Все потомки узла в порядке документа
    void appendDescendants(const Node* node, std::vector<XPathItem>& output)
    {
        for (const Node* cursor = firstChildNode(node); cursor != nullptr;
            cursor = nextInDocument(cursor, node))
            output.push_back(nodeItem(cursor));
    }

    // Дети узла (у атрибутов детей нет)
    void childAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        if (item.attribute != nullptr)
            return;
        for (const Node* child = firstChildNode(item.node); child != nullptr;
            child = child->getNextSibling())
            output.push_back(nodeItem(child));
    }
}
