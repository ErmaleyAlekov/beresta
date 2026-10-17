#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_axes.hpp"
#include "core/container_node.hpp"

namespace beresta::xpath::detail
{
    // Функция оси: дописывает узлы оси в результат
    using AxisFunction = void (*)(const XPathItem&, std::vector<XPathItem>&);

    /*
    ** Таблица осей в порядке перечисления Axis: выбор оси — одно
    ** обращение по индексу вместо длинного switch (паттерн Strategy).
    */
    static const AxisFunction AXIS_FUNCTIONS[] = {
        ancestorAxis, ancestorOrSelfAxis, attributeAxis, childAxis, descendantAxis,
        descendantOrSelfAxis, followingAxis, followingSiblingAxis, parentAxis,
        precedingAxis, precedingSiblingAxis, selfAxis
    };

    // Узлы оси для одного узла контекста
    std::vector<XPathItem> axisItems(Axis axis, const XPathItem& item)
    {
        std::vector<XPathItem> output;

        AXIS_FUNCTIONS[static_cast<std::size_t>(axis)](item, output);
        return output;
    }

    // Корень — самый верхний предок (документ или корень автономного дерева)
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
    XPathItem nodeItem(const Node* node)
    {
        XPathItem item;

        item.node = node;
        return item;
    }

    // Сам узел и все его потомки (основа сокращения "//")
    void descendantOrSelfAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        output.push_back(item);
        if (item.attribute == nullptr)
            appendDescendants(item.node, output);
    }

    // Родитель атрибута — его элемент; родитель узла — контейнер
    XPathItem parentItem(const XPathItem& item)
    {
        if (item.attribute != nullptr)
            return nodeItem(item.owner);
        if (item.node == nullptr || item.node->getParent() == nullptr)
            return XPathItem();
        return nodeItem(item.node->getParent());
    }
}
