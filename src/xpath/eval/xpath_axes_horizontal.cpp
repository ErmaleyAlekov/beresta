#include "xpath/internal/xpath_axes.hpp"
#include "core/container_node.hpp"

namespace beresta::xpath::detail
{
    // Правые соседи (у атрибутов соседей-узлов нет)
    // The right siblings (attributes have no sibling nodes)
    void followingSiblingAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        if (item.attribute != nullptr || item.node == nullptr)
            return;
        for (const Node* sibling = item.node->getNextSibling(); sibling != nullptr;
            sibling = sibling->getNextSibling())
            output.push_back(nodeItem(sibling));
    }

    // Левые соседи от ближайшего (обратная ось)
    // The left siblings from the nearest (a reverse axis)
    void precedingSiblingAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        if (item.attribute != nullptr || item.node == nullptr)
            return;
        for (const Node* sibling = item.node->getPreviousSibling(); sibling != nullptr;
            sibling = sibling->getPreviousSibling())
            output.push_back(nodeItem(sibling));
    }

    /*
    ** Всё, что после узла в порядке документа, кроме потомков. У атрибута
    ** «после» начинается сразу с детей его элемента.
    **
    ** Everything after the node in document order, except the descendants. For an attribute
    ** "after" starts right with the children of its element.
    */
    void followingAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        const Node* start = item.node;

        if (item.attribute != nullptr)
        {
            appendDescendants(item.owner, output);
            start = item.owner;
        }
        for (const Node* cursor = start; cursor != nullptr; cursor = cursor->getParent())
        {
            for (const Node* sibling = cursor->getNextSibling(); sibling != nullptr;
                sibling = sibling->getNextSibling())
            {
                output.push_back(nodeItem(sibling));
                appendDescendants(sibling, output);
            }
        }
    }

    // Поддерево в обратном порядке документа: сначала глубокие последние, корень — в конце
    // A subtree in reverse document order: the deep last ones first, the root at the end
    static void appendSubtreeReversed(const Node* node, std::vector<XPathItem>& output)
    {
        std::vector<XPathItem> forward;

        appendDescendants(node, forward);
        for (std::size_t index = forward.size(); index > 0; --index)
            output.push_back(forward[index - 1]);
        output.push_back(nodeItem(node));
    }

    /*
    ** Всё, что до узла в порядке документа, кроме предков, — от ближайшего
    ** к дальнему (обратная ось). Для атрибута — то же, что для его элемента.
    **
    ** Everything before the node in document order, except the ancestors — from the nearest
    ** to the farthest (a reverse axis). For an attribute — the same as for its element.
    */
    void precedingAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        const Node* start = item.node;

        if (item.attribute != nullptr)
            start = item.owner;
        for (const Node* cursor = start; cursor != nullptr; cursor = cursor->getParent())
        {
            for (const Node* sibling = cursor->getPreviousSibling(); sibling != nullptr;
                sibling = sibling->getPreviousSibling())
                appendSubtreeReversed(sibling, output);
        }
    }
}
