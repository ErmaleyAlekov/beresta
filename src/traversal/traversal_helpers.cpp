#include "traversal/tree_walker.hpp"
#include "core/container_node.hpp"

namespace beresta
{
    // Дети есть только у контейнеров (элементов и документа)
    // Only containers (elements and the document) have children
    Node* firstChildOf(Node& node)
    {
        if (node.getType() != NodeType::ELEMENT && node.getType() != NodeType::DOCUMENT)
            return nullptr;
        return static_cast<ContainerNode&>(node).getFirstChild();
    }

    // Последний ребёнок контейнера
    // The last child of a container
    Node* lastChildOf(Node& node)
    {
        if (node.getType() != NodeType::ELEMENT && node.getType() != NodeType::DOCUMENT)
            return nullptr;
        return static_cast<ContainerNode&>(node).getLastChild();
    }

    /*
    ** Следующий в порядке документа: первый ребёнок, иначе ближайший
    ** правый сосед самого узла или одного из его предков (не выше root).
    **
    ** The next in document order: the first child, otherwise the nearest
    ** right sibling of the node itself or of one of its ancestors (not above root).
    */
    Node* followingNode(Node& node, const Node& root)
    {
        Node* child = firstChildOf(node);

        if (child != nullptr)
            return child;
        for (Node* cursor = &node; cursor != nullptr && cursor != &root; cursor = cursor->getParent())
        {
            Node* sibling = cursor->getNextSibling();
            if (sibling != nullptr)
                return sibling;
        }
        return nullptr;
    }

    /*
    ** Предыдущий в порядке документа: самый глубокий последний потомок
    ** левого соседа, иначе родитель (сам root предыдущего не имеет).
    **
    ** The previous in document order: the deepest last descendant of the
    ** left sibling, otherwise the parent (root itself has no previous node).
    */
    Node* precedingNode(Node& node, const Node& root)
    {
        if (&node == &root)
            return nullptr;
        Node* sibling = node.getPreviousSibling();
        if (sibling == nullptr)
            return node.getParent();
        Node* deepest = sibling;
        while (lastChildOf(*deepest) != nullptr)
            deepest = lastChildOf(*deepest);
        return deepest;
    }
}
