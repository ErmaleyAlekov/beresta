#ifndef TREE_WALKER_HPP
#define TREE_WALKER_HPP

#include "core/container_node.hpp"
#include "traversal/node_filter.hpp"

namespace beresta
{
    /*
    ** TreeWalker — навигация по «логическому» дереву, из которого фильтр
    ** убрал лишние узлы (DOM Level 2 Traversal, как DOMTreeWalker в Xerces).
    ** Алгоритмы реализованы строго по спецификации DOM, включая различие
    ** SKIP (дети остаются видимыми) и REJECT (поддерево исчезает).
    ** Обход не выходит за пределы root. Фильтр — невладеющий указатель.
    **
    ** TreeWalker — navigation over the "logical" tree from which the filter
    ** removed unwanted nodes (DOM Level 2 Traversal, like DOMTreeWalker in Xerces).
    ** The algorithms follow the DOM specification strictly, including the difference
    ** between SKIP (the children stay visible) and REJECT (the subtree disappears).
    ** The traversal never leaves root. The filter is a non-owning pointer.
    */
    class TreeWalker
    {
    public:
        TreeWalker(Node& root, unsigned whatToShow = show::ALL_NODES,
            const NodeFilter* filter = nullptr) noexcept;

        Node& getRoot() const noexcept;
        Node& getCurrentNode() const noexcept;
        void setCurrentNode(Node& node) noexcept;

        // Каждый метод перемещает текущий узел и возвращает его (или nullptr, не двигаясь)
        // Each method moves the current node and returns it (or nullptr, without moving)
        Node* parentNode();
        Node* firstChild();
        Node* lastChild();
        Node* previousSibling();
        Node* nextSibling();
        Node* previousNode();
        Node* nextNode();

    private:
        // Решение для узла с учётом whatToShow и пользовательского фильтра
        // The decision for a node taking whatToShow and the user filter into account
        FilterResult filterNode(const Node& node) const;

        // Общий алгоритм firstChild / lastChild
        // The common algorithm of firstChild / lastChild
        Node* traverseChildren(bool first);

        // Общий алгоритм nextSibling / previousSibling
        // The common algorithm of nextSibling / previousSibling
        Node* traverseSiblings(bool next);

        // Подъём от узла до ближайшего соседа в направлении first (не выше текущего)
        // Climbs from a node to the nearest sibling in the first direction (not above the current one)
        Node* climbToSibling(Node* node, bool first) const;

        // Спуск к самому глубокому последнему потомку; true — итоговый узел принят
        // Descends to the deepest last descendant; true — the resulting node is accepted
        bool descendToLast(Node*& node) const;

        Node* _root;
        Node* _current;
        unsigned _whatToShow;
        const NodeFilter* _filter;
    };

    /*
    ** NodeIterator — плоский обход узлов в порядке документа (pre-order).
    ** REJECT для итератора равносилен SKIP (так определяет DOM).
    **
    ** NodeIterator — a flat traversal of nodes in document order (pre-order).
    ** REJECT for the iterator is the same as SKIP (that is how DOM defines it).
    */
    class NodeIterator
    {
    public:
        NodeIterator(Node& root, unsigned whatToShow = show::ALL_NODES,
            const NodeFilter* filter = nullptr) noexcept;

        // Следующий / предыдущий подходящий узел или nullptr
        // The next / previous matching node or nullptr
        Node* nextNode();
        Node* previousNode();

        Node& getReferenceNode() const noexcept;

    private:
        bool accepts(const Node& node) const;

        Node* _root;
        Node* _reference;
        bool _beforeReference;
        unsigned _whatToShow;
        const NodeFilter* _filter;
    };

    // ---- Помощники обхода (traversal_helpers.cpp) ----
    // ---- Traversal helpers (traversal_helpers.cpp) ----

    // Первый / последний ребёнок узла любого типа (у листьев — nullptr)
    // The first / last child of a node of any type (for leaves — nullptr)
    Node* firstChildOf(Node& node);
    Node* lastChildOf(Node& node);

    // Следующий / предыдущий узел в порядке документа в пределах root
    // The next / previous node in document order within root
    Node* followingNode(Node& node, const Node& root);
    Node* precedingNode(Node& node, const Node& root);
}

#endif
