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
        Node* parentNode();
        Node* firstChild();
        Node* lastChild();
        Node* previousSibling();
        Node* nextSibling();
        Node* previousNode();
        Node* nextNode();

    private:
        // Решение для узла с учётом whatToShow и пользовательского фильтра
        FilterResult filterNode(const Node& node) const;

        // Общий алгоритм firstChild / lastChild
        Node* traverseChildren(bool first);

        // Общий алгоритм nextSibling / previousSibling
        Node* traverseSiblings(bool next);

        // Подъём от узла до ближайшего соседа в направлении first (не выше текущего)
        Node* climbToSibling(Node* node, bool first) const;

        // Спуск к самому глубокому последнему потомку; true — итоговый узел принят
        bool descendToLast(Node*& node) const;

        Node* _root;
        Node* _current;
        unsigned _whatToShow;
        const NodeFilter* _filter;
    };

    /*
    ** NodeIterator — плоский обход узлов в порядке документа (pre-order).
    ** REJECT для итератора равносилен SKIP (так определяет DOM).
    */
    class NodeIterator
    {
    public:
        NodeIterator(Node& root, unsigned whatToShow = show::ALL_NODES,
            const NodeFilter* filter = nullptr) noexcept;

        // Следующий / предыдущий подходящий узел или nullptr
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

    // Первый / последний ребёнок узла любого типа (у листьев — nullptr)
    Node* firstChildOf(Node& node);
    Node* lastChildOf(Node& node);

    // Следующий / предыдущий узел в порядке документа в пределах root
    Node* followingNode(Node& node, const Node& root);
    Node* precedingNode(Node& node, const Node& root);
}

#endif
