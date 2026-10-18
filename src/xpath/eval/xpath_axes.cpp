#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_axes.hpp"
#include "core/container_node.hpp"

namespace beresta::xpath::detail
{
    // Функция оси: дописывает узлы оси в результат
    // An axis function: appends the axis nodes to the result
    using AxisFunction = void (*)(const XPathItem&, std::vector<XPathItem>&);

    /*
    ** Таблица осей в порядке перечисления Axis: выбор оси — одно
    ** обращение по индексу вместо длинного switch (паттерн Strategy).
    **
    ** The axis table in the order of the Axis enumeration: choosing an axis is one
    ** indexed access instead of a long switch (the Strategy pattern).
    */
    static const AxisFunction AXIS_FUNCTIONS[] = {
        ancestorAxis, ancestorOrSelfAxis, attributeAxis, childAxis, descendantAxis,
        descendantOrSelfAxis, followingAxis, followingSiblingAxis, parentAxis,
        precedingAxis, precedingSiblingAxis, selfAxis, namespaceAxis
    };

    // Узлы оси для одного узла контекста
    // The axis nodes for one context node
    void appendAxisItems(Axis axis, const XPathItem& item, std::vector<XPathItem>& output)
    {
        AXIS_FUNCTIONS[static_cast<std::size_t>(axis)](item, output);
    }

    // Корень — самый верхний предок (документ или корень автономного дерева)
    // The root is the topmost ancestor (the document or the root of a standalone tree)
    XPathItem rootOf(const XPathItem& item)
    {
        const Node* node = item.node;

        if (item.attribute != nullptr)
            node = item.owner;
        while (node != nullptr && node->getParent() != nullptr)
            node = node->getParent();
        return nodeItem(node);
    }

    // Обычный узел без атрибута
    // An ordinary node without an attribute
    XPathItem nodeItem(const Node* node)
    {
        XPathItem item;

        item.node = node;
        return item;
    }

    // Сам узел и все его потомки (основа сокращения "//")
    // The node itself and all its descendants (the basis of the "//" abbreviation)
    void descendantOrSelfAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        output.push_back(item);
        if (item.attribute == nullptr)
            appendDescendants(item.node, output);
    }

    // Родитель атрибута — его элемент; родитель узла — контейнер
    // The parent of an attribute is its element; the parent of a node is the container
    XPathItem parentItem(const XPathItem& item)
    {
        if (item.attribute != nullptr)
            return nodeItem(item.owner);
        if (item.node == nullptr || item.node->getParent() == nullptr)
            return XPathItem();
        return nodeItem(item.node->getParent());
    }
}
