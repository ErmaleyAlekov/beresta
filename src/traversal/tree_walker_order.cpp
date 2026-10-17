#include "traversal/tree_walker.hpp"

namespace beresta
{
    /*
    ** Предыдущий в порядке документа (алгоритм DOM): у левого соседа
    ** спускаемся к самому глубокому последнему видимому потомку, иначе
    ** берём родителя, если он принят фильтром.
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
