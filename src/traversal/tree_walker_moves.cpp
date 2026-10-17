#include "traversal/tree_walker.hpp"

namespace beresta
{
    // Ближайший принятый предок, не выше корня обхода
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
    Node* TreeWalker::firstChild()
    {
        return traverseChildren(true);
    }

    // Последний видимый ребёнок
    Node* TreeWalker::lastChild()
    {
        return traverseChildren(false);
    }

    // Предыдущий видимый сосед
    Node* TreeWalker::previousSibling()
    {
        return traverseSiblings(false);
    }

    // Следующий видимый сосед
    Node* TreeWalker::nextSibling()
    {
        return traverseSiblings(true);
    }
}
