#include "traversal/tree_walker.hpp"

namespace beresta
{
    // Итератор стоит «перед» корнем: первый nextNode() вернёт сам корень (если он подходит)
    NodeIterator::NodeIterator(Node& root, unsigned whatToShow, const NodeFilter* filter) noexcept
        : _root(&root), _reference(&root), _beforeReference(true),
          _whatToShow(whatToShow), _filter(filter)
    {
    }

    // Узел подходит, если виден по маске и не отвергнут фильтром
    bool NodeIterator::accepts(const Node& node) const
    {
        if ((_whatToShow & showMaskFor(node.getType())) == 0)
            return false;
        return _filter == nullptr || _filter->acceptNode(node) == FilterResult::ACCEPT;
    }

    // Вперёд по порядку документа до первого подходящего узла
    Node* NodeIterator::nextNode()
    {
        Node* node = _reference;
        bool before = _beforeReference;

        while (true)
        {
            if (!before)
                node = followingNode(*node, *_root);
            before = false;
            if (node == nullptr)
                return nullptr;
            if (!accepts(*node))
                continue;
            _reference = node;
            _beforeReference = false;
            return node;
        }
    }

    // Назад по порядку документа до первого подходящего узла
    Node* NodeIterator::previousNode()
    {
        Node* node = _reference;
        bool before = _beforeReference;

        while (true)
        {
            if (before)
                node = precedingNode(*node, *_root);
            before = true;
            if (node == nullptr)
                return nullptr;
            if (!accepts(*node))
                continue;
            _reference = node;
            _beforeReference = true;
            return node;
        }
    }

    // Опорный узел итератора
    Node& NodeIterator::getReferenceNode() const noexcept
    {
        return *_reference;
    }
}
