#include "traversal/tree_walker.hpp"

namespace beresta
{
    /*
    ** Предыдущий в порядке документа (алгоритм DOM): у левого соседа
    ** спускаемся к самому глубокому последнему видимому потомку, иначе
    ** берём родителя, если он принят фильтром.
    **
    ** The previous in document order (the DOM algorithm): at the left sibling
    ** descend to the deepest last visible descendant, otherwise
    ** take the parent if the filter accepts it.
    */
    Node* TreeWalker::previousNode()
    {
        Node* node = _current;

        while (node != _root)
        {
            Node* sibling = node->getPreviousSibling();
            while (sibling != nullptr)
            {
                node = sibling;
                if (descendToLast(node))
                {
                    _current = node;
                    return node;
                }
                sibling = node->getPreviousSibling();
            }
            if (node == _root || node->getParent() == nullptr)
                return nullptr;
            node = node->getParent();
            if (filterNode(*node) == FilterResult::ACCEPT)
            {
                _current = node;
                return node;
            }
        }
        return nullptr;
    }

    // Следующий правый сосед узла или его предков в пределах корня
    // The next right sibling of the node or its ancestors within the root
    static Node* nextOutside(Node* node, const Node* root)
    {
        for (Node* cursor = node; cursor != nullptr; cursor = cursor->getParent())
        {
            if (cursor == root)
                return nullptr;
            if (cursor->getNextSibling() != nullptr)
                return cursor->getNextSibling();
        }
        return nullptr;
    }

    /*
    ** Следующий в порядке документа (алгоритм DOM): вглубь через первых
    ** детей (кроме отвергнутых поддеревьев), затем вправо и вверх.
    **
    ** The next in document order (the DOM algorithm): down through the first
    ** children (except rejected subtrees), then right and up.
    */
    Node* TreeWalker::nextNode()
    {
        Node* node = _current;
        FilterResult result = FilterResult::ACCEPT;

        while (true)
        {
            while (result != FilterResult::REJECT && firstChildOf(*node) != nullptr)
            {
                node = firstChildOf(*node);
                result = filterNode(*node);
                if (result == FilterResult::ACCEPT)
                {
                    _current = node;
                    return node;
                }
            }
            node = nextOutside(node, _root);
            if (node == nullptr)
                return nullptr;
            result = filterNode(*node);
            if (result == FilterResult::ACCEPT)
            {
                _current = node;
                return node;
            }
        }
    }

    // Пока узел не отвергнут и у него есть дети — спускаемся к последнему ребёнку
    // While the node is not rejected and has children — descend to the last child
    bool TreeWalker::descendToLast(Node*& node) const
    {
        FilterResult result = filterNode(*node);

        while (result != FilterResult::REJECT && lastChildOf(*node) != nullptr)
        {
            node = lastChildOf(*node);
            result = filterNode(*node);
        }
        return result == FilterResult::ACCEPT;
    }
}
