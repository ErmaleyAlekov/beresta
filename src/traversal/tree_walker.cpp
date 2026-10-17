#include "traversal/tree_walker.hpp"

namespace beresta
{
    // Обход начинается с корня
    TreeWalker::TreeWalker(Node& root, unsigned whatToShow, const NodeFilter* filter) noexcept
        : _root(&root), _current(&root), _whatToShow(whatToShow), _filter(filter)
    {
    }

    // Корень обхода
    Node& TreeWalker::getRoot() const noexcept
    {
        return *_root;
    }

    // Текущий узел
    Node& TreeWalker::getCurrentNode() const noexcept
    {
        return *_current;
    }

    // Текущим можно сделать любой узел (как в DOM)
    void TreeWalker::setCurrentNode(Node& node) noexcept
    {
        _current = &node;
    }

    // Сначала маска whatToShow, затем пользовательский фильтр
    FilterResult TreeWalker::filterNode(const Node& node) const
    {
        if ((_whatToShow & showMaskFor(node.getType())) == 0)
            return FilterResult::SKIP;
        if (_filter == nullptr)
            return FilterResult::ACCEPT;
        return _filter->acceptNode(node);
    }
}
