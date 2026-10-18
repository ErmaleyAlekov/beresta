#include "xpath/internal/xpath_axes.hpp"
#include "core/container_node.hpp"

namespace beresta::xpath::detail
{
    // Дети есть только у элементов и документа
    // Only elements and the document have children
    const Node* firstChildNode(const Node* node)
    {
        if (node == nullptr)
            return nullptr;
        if (node->getType() != NodeType::ELEMENT && node->getType() != NodeType::DOCUMENT)
            return nullptr;
        return static_cast<const ContainerNode*>(node)->getFirstChild();
    }

    // Последний ребёнок контейнера
    // The last child of a container
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
    **
    ** Document order without recursion: first down (the first child),
    ** then right, and if there is no sibling — up, but not above root.
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
    // All descendants of the node in document order
    void appendDescendants(const Node* node, std::vector<XPathItem>& output)
    {
        for (const Node* cursor = firstChildNode(node); cursor != nullptr;
            cursor = nextInDocument(cursor, node))
            output.push_back(nodeItem(cursor));
    }

    // Дети узла (у атрибутов детей нет)
    // The children of the node (attributes have no children)
    void childAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        if (item.attribute != nullptr)
            return;
        for (const Node* child = firstChildNode(item.node); child != nullptr;
            child = child->getNextSibling())
            output.push_back(nodeItem(child));
    }
}
