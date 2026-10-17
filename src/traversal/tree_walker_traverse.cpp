#include "traversal/tree_walker.hpp"

namespace beresta
{
    // Ребёнок в выбранном направлении (первый / последний)
    static Node* childInDirection(Node& node, bool first)
    {
        if (first)
            return firstChildOf(node);
        return lastChildOf(node);
    }

    // Сосед в выбранном направлении (следующий / предыдущий)
    static Node* siblingInDirection(Node& node, bool next)
    {
        if (next)
            return node.getNextSibling();
        return node.getPreviousSibling();
    }

    /*
    ** Алгоритм «traverse children» из DOM: принятый узел возвращается,
    ** в SKIP-узел спускаемся (его дети видимы), REJECT-узел обходим
    ** целиком; если ветка закончилась — поднимаемся, но не выше текущего.
    */
    Node* TreeWalker::traverseChildren(bool first)
    {
        Node* node = childInDirection(*_current, first);

        while (node != nullptr)
        {
            FilterResult result = filterNode(*node);
            if (result == FilterResult::ACCEPT)
            {
                _current = node;
                return node;
            }
            Node* child = childInDirection(*node, first);
            if (result == FilterResult::SKIP && child != nullptr)
            {
                node = child;
                continue;
            }
            node = climbToSibling(node, first);
        }
        return nullptr;
    }

    // Поднимаемся, пока у узла нет соседа; выше корня и текущего узла не идём
    Node* TreeWalker::climbToSibling(Node* node, bool first) const
    {
        while (node != nullptr && siblingInDirection(*node, first) == nullptr)
        {
            node = node->getParent();
            if (node == nullptr || node == _root || node == _current)
                return nullptr;
        }
        if (node == nullptr)
            return nullptr;
        return siblingInDirection(*node, first);
    }

    /*
    ** Алгоритм «traverse siblings» из DOM: перебор соседей с заходом
    ** внутрь пропущенных (SKIP) узлов; подъём к родителю прекращается,
    ** если родитель сам виден — его соседи уже не «наши» соседи.
    */
    Node* TreeWalker::traverseSiblings(bool next)
    {
        Node* node = _current;

        if (node == _root)
            return nullptr;
        while (true)
        {
            Node* sibling = siblingInDirection(*node, next);
            while (sibling != nullptr)
            {
                node = sibling;
                FilterResult result = filterNode(*node);
                if (result == FilterResult::ACCEPT)
                {
                    _current = node;
                    return node;
                }
                sibling = childInDirection(*node, next);
                if (result == FilterResult::REJECT || sibling == nullptr)
                    sibling = siblingInDirection(*node, next);
            }
            node = node->getParent();
            if (node == nullptr || node == _root || filterNode(*node) == FilterResult::ACCEPT)
                return nullptr;
        }
    }
}
