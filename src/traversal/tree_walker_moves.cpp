#include "traversal/tree_walker.hpp"

namespace beresta
{
    // Ближайший принятый предок, не выше корня обхода
    // The nearest accepted ancestor, not above the traversal root
    Node* TreeWalker::parentNode()
    {
        Node* node = _current;

        while (node != nullptr && node != _root)
        {
            node = node->getParent();
            if (node != nullptr && filterNode(*node) == FilterResult::ACCEPT)
            {
                _current = node;
                return node;
            }
        }
        return nullptr;
    }

    // Первый видимый ребёнок «логического» дерева
    // The first visible child of the "logical" tree
    Node* TreeWalker::firstChild()
    {
        return traverseChildren(true);
    }

    // Последний видимый ребёнок
    // The last visible child
    Node* TreeWalker::lastChild()
    {
        return traverseChildren(false);
    }

    // Предыдущий видимый сосед
    // The previous visible sibling
    Node* TreeWalker::previousSibling()
    {
        return traverseSiblings(false);
    }

    // Следующий видимый сосед
    // The next visible sibling
    Node* TreeWalker::nextSibling()
    {
        return traverseSiblings(true);
    }
}
