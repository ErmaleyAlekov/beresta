#ifndef XPATH_AXES_HPP
#define XPATH_AXES_HPP

#include "xpath/xpath_value.hpp"

#include <vector>

namespace beresta::xpath::detail
{
    // ---- Узлы модели XPath (xpath_axes.cpp) ----
    // ---- XPath model nodes (xpath_axes.cpp) ----

    // Узел-не-атрибут
    // A non-attribute node
    XPathItem nodeItem(const Node* node);

    // Родитель в модели XPath: у атрибута — элемент-владелец
    // The parent in the XPath model: for an attribute — the owner element
    XPathItem parentItem(const XPathItem& item);

    // ---- Обход дерева (xpath_axes_tree.cpp) ----
    // ---- Tree traversal (xpath_axes_tree.cpp) ----

    // Первый / последний ребёнок (у листьев — nullptr)
    // The first / last child (for leaves — nullptr)
    const Node* firstChildNode(const Node* node);
    const Node* lastChildNode(const Node* node);

    // Следующий узел в порядке документа внутри поддерева root (без рекурсии)
    // The next node in document order inside the subtree of root (without recursion)
    const Node* nextInDocument(const Node* node, const Node* root);

    // Все потомки в порядке документа
    // All descendants in document order
    void appendDescendants(const Node* node, std::vector<XPathItem>& output);

    /*
    ** Функции осей: дописывают узлы оси в output в порядке оси.
    ** Объединены в таблицу, индексируемую значением Axis (xpath_axes.cpp).
    **
    ** Axis functions: append the axis nodes to output in axis order.
    ** They are collected in a table indexed by the Axis value (xpath_axes.cpp).
    */
    void selfAxis(const XPathItem& item, std::vector<XPathItem>& output);
    void childAxis(const XPathItem& item, std::vector<XPathItem>& output);
    void parentAxis(const XPathItem& item, std::vector<XPathItem>& output);
    void ancestorAxis(const XPathItem& item, std::vector<XPathItem>& output);
    void ancestorOrSelfAxis(const XPathItem& item, std::vector<XPathItem>& output);
    void descendantAxis(const XPathItem& item, std::vector<XPathItem>& output);
    void descendantOrSelfAxis(const XPathItem& item, std::vector<XPathItem>& output);
    void followingSiblingAxis(const XPathItem& item, std::vector<XPathItem>& output);
    void precedingSiblingAxis(const XPathItem& item, std::vector<XPathItem>& output);
    void followingAxis(const XPathItem& item, std::vector<XPathItem>& output);
    void precedingAxis(const XPathItem& item, std::vector<XPathItem>& output);
    void attributeAxis(const XPathItem& item, std::vector<XPathItem>& output);

    // Узлы пространств имён элемента: все действующие объявления + xml (xpath_axes_namespace.cpp)
    // The namespace nodes of an element: all declarations in scope + xml (xpath_axes_namespace.cpp)
    void namespaceAxis(const XPathItem& item, std::vector<XPathItem>& output);
}

#endif
