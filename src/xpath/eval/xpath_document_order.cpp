#include "xpath/internal/xpath_eval.hpp"
#include "core/container_node.hpp"

#include <algorithm>
#include <cstdint>

namespace beresta::xpath::detail
{
    using OrderKey = std::vector<std::size_t>;

    /*
    ** Хвост ключа для атрибута и узла пространства имён (в обратном порядке):
    ** (0, 1, индекс атрибута) и (0, 0, адрес объявления). По XPath узлы
    ** пространств имён идут после своего элемента, но раньше атрибутов;
    ** их взаимный порядок не задан стандартом — адрес делает его стабильным.
    **
    ** The key tail for an attribute and a namespace node (in reverse order):
    ** (0, 1, attribute index) and (0, 0, declaration address). By XPath namespace
    ** nodes come after their element but before the attributes;
    ** their mutual order is not defined by the standard — the address makes it stable.
    */
    static void appendAttributeKey(const XPathItem& item, OrderKey& reversed)
    {
        if (item.isNamespace)
        {
            reversed.push_back(static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(item.attribute)));
            reversed.push_back(0);
        }
        else
        {
            reversed.push_back(static_cast<std::size_t>(item.attribute - item.owner->getAttributes().data()));
            reversed.push_back(1);
        }
        reversed.push_back(0);
    }

    /*
    ** Ключ порядка документа: адрес корня, затем для каждого уровня пара
    ** (1, индекс ребёнка). Атрибуты и узлы пространств имён получают в конце
    ** тройку с нулём впереди: так они идут после элемента, но раньше детей.
    ** Лексикографическое сравнение ключей = сравнение в порядке документа.
    **
    ** The document order key: the root address, then for each level the pair
    ** (1, child index). Attributes and namespace nodes get at the end
    ** a triple starting with zero: so they come after the element but before the children.
    ** A lexicographic comparison of the keys = a comparison in document order.
    */
    OrderKey documentOrderKey(const XPathItem& item)
    {
        const Node* node = item.node;
        OrderKey reversed;

        if (item.attribute != nullptr)
        {
            node = item.owner;
            appendAttributeKey(item, reversed);
        }
        while (node != nullptr && node->getParent() != nullptr)
        {
            reversed.push_back(node->getIndexInParent());
            reversed.push_back(1);
            node = node->getParent();
        }
        reversed.push_back(static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(node)));
        std::reverse(reversed.begin(), reversed.end());
        return reversed;
    }

    // Сравнитель индексов по заранее вычисленным ключам (функтор вместо лямбды)
    // An index comparator by precomputed keys (a functor instead of a lambda)
    struct KeyLess
    {
        const std::vector<OrderKey>* keys;

        bool operator()(std::size_t left, std::size_t right) const;
    };

    // Сравнение двух позиций по их ключам
    // Compares two positions by their keys
    bool KeyLess::operator()(std::size_t left, std::size_t right) const
    {
        return (*keys)[left] < (*keys)[right];
    }

    // Один и тот же узел модели XPath
    // The same node of the XPath model
    static bool sameItem(const XPathItem& left, const XPathItem& right) noexcept
    {
        return left.node == right.node && left.attribute == right.attribute && left.owner == right.owner
            && left.isNamespace == right.isNamespace;
    }

    /*
    ** Ключи вычисляются один раз на элемент (O(глубина)), затем сортируются
    ** индексы; повторы после сортировки стоят рядом и удаляются за один проход.
    **
    ** The keys are computed once per item (O(depth)), then the indexes are
    ** sorted; duplicates are adjacent after sorting and are removed in one pass.
    */
    void sortDocumentOrder(std::vector<XPathItem>& items)
    {
        if (items.size() < 2 || isDocumentOrdered(items))
            return;
        std::vector<OrderKey> keys;
        std::vector<std::size_t> order(items.size());
        keys.reserve(items.size());
        for (std::size_t index = 0; index < items.size(); ++index)
        {
            keys.push_back(documentOrderKey(items[index]));
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
