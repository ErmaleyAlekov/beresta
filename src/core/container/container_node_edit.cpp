#include "core/container_node.hpp"
#include "core/text.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Отсоединение по ссылке: находим индекс и делегируем removeChild()
    std::unique_ptr<Node> ContainerNode::detachChild(const Node& child)
    {
        std::size_t index = indexOf(&child);

        if (index == npos)
            throw DomException(DomErrorCode::NOT_FOUND,
                "узел не является непосредственным ребёнком этого контейнера");
        return removeChild(index);
    }

    /*
    ** Замена: старый узел извлекается, новый встаёт на его место.
    ** Если новый узел отвергнут проверками, старый возвращается обратно —
    ** дерево остаётся прежним, а новый узел — у вызывающего (строгая гарантия).
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
    ** сливаются, вложенные контейнеры нормализуются рекурсивно.
    */
    void ContainerNode::normalize()
    {
        std::size_t index = 0;

        while (index < _children.size())
        {
            Node& child = *_children[index];
            if (child.getType() == NodeType::TEXT)
            {
                mergeTextRun(index);
                if (static_cast<Text&>(*_children[index]).getValue().empty())
                {
                    removeChild(index);
                    continue;
                }
            }
            else if (child.getType() == NodeType::ELEMENT)
                static_cast<ContainerNode&>(child).normalize();
            ++index;
        }
    }

    // Пока правый сосед — тоже Text, переносим его значение и удаляем его
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
    void ContainerNode::reindexFrom(std::size_t start) noexcept
    {
        for (std::size_t index = start; index < _children.size(); ++index)
            _children[index]->setSiblingIndex(index);
    }
}
