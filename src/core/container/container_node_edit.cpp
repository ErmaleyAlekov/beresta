#include "core/container_node.hpp"
#include "i18n/i18n.hpp"
#include "core/text.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Отсоединение по ссылке: находим индекс и делегируем removeChild()
    // Detaching by reference: find the index and delegate to removeChild()
    std::unique_ptr<Node> ContainerNode::detachChild(const Node& child)
    {
        std::size_t index = indexOf(&child);

        if (index == npos)
            throw DomException(DomErrorCode::NOT_FOUND,
                i18n::format(i18n::Message::NOT_A_DIRECT_CHILD));
        return removeChild(index);
    }

    /*
    ** Замена: старый узел извлекается, новый встаёт на его место.
    ** Если новый узел отвергнут проверками, старый возвращается обратно —
    ** дерево остаётся прежним, а новый узел — у вызывающего (строгая гарантия).
    **
    ** Replacement: the old node is extracted, the new one takes its place.
    ** If the new node is rejected by the checks, the old one is put back —
    ** the tree stays the same, and the new node stays with the caller (the strong guarantee).
    */
    std::unique_ptr<Node> ContainerNode::replaceChild(std::size_t index,
        std::unique_ptr<Node>&& child)
    {
        std::unique_ptr<Node> previous = removeChild(index);

        try
        {
            insertChild(index, std::move(child));
        }
        catch (...)
        {
            insertChild(index, std::move(previous));
            throw;
        }
        return previous;
    }

    /*
    ** Нормализация поддерева: пустые тексты удаляются, соседние тексты
    ** сливаются. Вложенные элементы откладываются в явный стек, а не
    ** обрабатываются рекурсией, — глубина дерева не ограничена стеком.
    **
    ** Normalizing a subtree: empty texts are removed, adjacent texts are
    ** merged. Nested elements are put aside on an explicit stack instead of
    ** being processed recursively — the tree depth is not limited by the stack.
    */
    void ContainerNode::normalize()
    {
        std::vector<ContainerNode*> pending{this};

        while (!pending.empty())
        {
            ContainerNode* current = pending.back();
            pending.pop_back();
            std::size_t index = 0;
            while (index < current->_children.size())
            {
                Node& child = *current->_children[index];
                if (child.getType() == NodeType::TEXT)
                {
                    current->mergeTextRun(index);
                    if (static_cast<Text&>(*current->_children[index]).getValue().empty())
                    {
                        current->removeChild(index);
                        continue;
                    }
                }
                else if (child.getType() == NodeType::ELEMENT)
                    pending.push_back(asContainer(child));
                ++index;
            }
        }
    }

    // Пока правый сосед — тоже Text, переносим его значение и удаляем его
    // While the right sibling is also a Text, move its value and remove it
    void ContainerNode::mergeTextRun(std::size_t index)
    {
        Text& target = static_cast<Text&>(*_children[index]);

        while (index + 1 < _children.size()
            && _children[index + 1]->getType() == NodeType::TEXT)
        {
            const Text& next = static_cast<const Text&>(*_children[index + 1]);
            target.setValue(target.getValue() + next.getValue());
            removeChild(index + 1);
        }
    }

    // Индексы детей начиная с start выравниваются по их позиции в векторе
    // The indexes of the children starting from start are aligned with their position in the vector
    void ContainerNode::reindexFrom(std::size_t start) noexcept
    {
        for (std::size_t index = start; index < _children.size(); ++index)
            _children[index]->setSiblingIndex(index);
    }
}
