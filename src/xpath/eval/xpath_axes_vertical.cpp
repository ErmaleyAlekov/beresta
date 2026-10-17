#include "xpath/internal/xpath_axes.hpp"
#include "core/container_node.hpp"

namespace beresta::xpath::detail
{
    // Сам узел
    void selfAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        output.push_back(item);
    }

    // Родитель (у корня родителя нет)
    void parentAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        XPathItem parent = parentItem(item);

        if (parent.node != nullptr)
            output.push_back(parent);
    }

    // Предки от ближайшего к корню (обратная ось)
    void ancestorAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        XPathItem parent = parentItem(item);

        while (parent.node != nullptr)
        {
            output.push_back(parent);
            parent = parentItem(parent);
        }
    }

    // Сам узел и его предки
    void ancestorOrSelfAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        output.push_back(item);
        ancestorAxis(item, output);
    }

    // Потомки в порядке документа (у атрибутов потомков нет)
    void descendantAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        if (item.attribute == nullptr)
            appendDescendants(item.node, output);
    }
}
