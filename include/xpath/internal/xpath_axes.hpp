#ifndef XPATH_AXES_HPP
#define XPATH_AXES_HPP

#include "xpath/xpath_value.hpp"

#include <vector>

namespace beresta::xpath::detail
{
    // ---- Узлы модели XPath (xpath_axes.cpp) ----

    // Узел-не-атрибут
    XPathItem nodeItem(const Node* node);

    // Родитель в модели XPath: у атрибута — элемент-владелец
    XPathItem parentItem(const XPathItem& item);

    // ---- Обход дерева (xpath_axes_tree.cpp) ----

    // Первый / последний ребёнок (у листьев — nullptr)
    const Node* firstChildNode(const Node* node);
    const Node* lastChildNode(const Node* node);

    // Следующий узел в порядке документа внутри поддерева root (без рекурсии)
    const Node* nextInDocument(const Node* node, const Node* root);

    // Все потомки в порядке документа
    void appendDescendants(const Node* node, std::vector<XPathItem>& output);

    /*
    ** Функции осей: дописывают узлы оси в output в порядке оси.
    ** Объединены в таблицу, индексируемую значением Axis (xpath_axes.cpp).
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
}

#endif
