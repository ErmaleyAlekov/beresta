#include "xpath/internal/xpath_eval.hpp"
#include "core/node.hpp"

namespace beresta::xpath::detail
{
    /*
    ** Строго ли left раньше right в порядке документа. Частый случай —
    ** два обычных узла с общим родителем (дети одного элемента): достаточно
    ** сравнить их индексы, без построения ключей и выделения памяти.
    **
    ** Whether left is strictly before right in document order. The frequent case —
    ** two ordinary nodes with a common parent (children of one element): comparing
    ** their indexes is enough, without building keys and allocating memory.
    */
    static bool strictlyBefore(const XPathItem& left, const XPathItem& right)
    {
        bool plain = left.attribute == nullptr && right.attribute == nullptr;

        if (plain && left.node != nullptr && right.node != nullptr
            && left.node->getParent() != nullptr && left.node->getParent() == right.node->getParent())
            return left.node->getIndexInParent() < right.node->getIndexInParent();
        return documentOrderKey(left) < documentOrderKey(right);
    }

    /*
    ** Проверка «уже упорядочено»: результаты шагов XPath чаще всего уже
    ** идут по порядку документа, и тогда полная сортировка (ключ на каждый
    ** узел + stable_sort) не нужна. Строгое сравнение заодно гарантирует
    ** отсутствие повторов.
    **
    ** The "already ordered" check: XPath step results are most often already in
    ** document order, and then the full sort (a key per node + stable_sort) is
    ** not needed. The strict comparison also guarantees there are no duplicates.
    */
    bool isDocumentOrdered(const std::vector<XPathItem>& items)
    {
        for (std::size_t index = 1; index < items.size(); ++index)
        {
            if (!strictlyBefore(items[index - 1], items[index]))
                return false;
        }
        return true;
    }
}
