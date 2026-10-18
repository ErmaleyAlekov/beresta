#include "traversal/tree_walker.hpp"

namespace beresta
{
    // Обход начинается с корня
    // The traversal starts at the root
    TreeWalker::TreeWalker(Node& root, unsigned whatToShow, const NodeFilter* filter) noexcept
        : _root(&root), _current(&root), _whatToShow(whatToShow), _filter(filter)
    {
    }

    // Корень обхода
    // The traversal root
    Node& TreeWalker::getRoot() const noexcept
    {
        return *_root;
    }

    // Текущий узел
    // The current node
    Node& TreeWalker::getCurrentNode() const noexcept
    {
        return *_current;
    }

    // Текущим можно сделать любой узел (как в DOM)
    // Any node can be made current (as in DOM)
    void TreeWalker::setCurrentNode(Node& node) noexcept
    {
        _current = &node;
    }

    // Сначала маска whatToShow, затем пользовательский фильтр
    // First the whatToShow mask, then the user filter
    FilterResult TreeWalker::filterNode(const Node& node) const
    {
        if ((_whatToShow & showMaskFor(node.getType())) == 0)
            return FilterResult::SKIP;
        if (_filter == nullptr)
            return FilterResult::ACCEPT;
        return _filter->acceptNode(node);
    }
}
