#include "xpath/internal/xpath_eval.hpp"
#include "core/container_node.hpp"

#include <algorithm>
#include <cstdint>

namespace beresta::xpath::detail
{
    using OrderKey = std::vector<std::size_t>;

    /*
    ** Ключ порядка документа: адрес корня, затем для каждого уровня пара
    ** (1, индекс ребёнка). Атрибут получает в конце пару (0, индекс атрибута):
    ** так атрибуты идут после своего элемента, но раньше его детей.
    ** Лексикографическое сравнение ключей = сравнение в порядке документа.
    */
    static OrderKey orderKey(const XPathItem& item)
    {
        const Node* node = item.node;
        OrderKey reversed;

        if (item.attribute != nullptr)
        {
            node = item.owner;
            reversed.push_back(static_cast<std::size_t>(item.attribute - item.owner->getAttributes().data()));
            reversed.push_back(0);
        }
        while (node != nullptr && node->getParent() != nullptr)
        {
            reversed.push_back(node->getIndexInParent());
            reversed.push_back(1);
            node = node->getParent();
        }
        reversed.push_back(static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(node)));
        return OrderKey(reversed.rbegin(), reversed.rend());
    }

    // Сравнитель индексов по заранее вычисленным ключам (функтор вместо лямбды)
    struct KeyLess
    {
        const std::vector<OrderKey>* keys;

        bool operator()(std::size_t left, std::size_t right) const;
    };

    // Сравнение двух позиций по их ключам
    bool KeyLess::operator()(std::size_t left, std::size_t right) const
    {
        return (*keys)[left] < (*keys)[right];
    }

    // Один и тот же узел модели XPath
    static bool sameItem(const XPathItem& left, const XPathItem& right) noexcept
    {
        return left.node == right.node && left.attribute == right.attribute;
    }

    /*
    ** Ключи вычисляются один раз на элемент (O(глубина)), затем сортируются
    ** индексы; повторы после сортировки стоят рядом и удаляются за один проход.
    */
    void sortDocumentOrder(std::vector<XPathItem>& items)
    {
        if (items.size() < 2)
            return;
        std::vector<OrderKey> keys;
        std::vector<std::size_t> order(items.size());
        keys.reserve(items.size());
        for (std::size_t index = 0; index < items.size(); ++index)
        {
            keys.push_back(orderKey(items[index]));
            order[index] = index;
        }
        std::stable_sort(order.begin(), order.end(), KeyLess{&keys});
        std::vector<XPathItem> sorted;
        sorted.reserve(items.size());
        for (std::size_t index : order)
        {
            if (sorted.empty() || !sameItem(sorted.back(), items[index]))
                sorted.push_back(items[index]);
        }
        items = std::move(sorted);
    }
}
