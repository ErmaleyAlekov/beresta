#include "core/container_node.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Дети уничтожаются автоматически вместе с вектором unique_ptr
    ContainerNode::~ContainerNode()
    {
    }

    // Вставка в конец — частный случай вставки по индексу
    Node* ContainerNode::appendChild(std::unique_ptr<Node>&& child)
    {
        return insertChild(_children.size(), std::move(child));
    }

    /*
    ** Вставка по индексу. Сначала все проверки (индекс, структура, тип),
    ** и только потом изменение состояния — при ошибке дерево не меняется.
    ** После вставки индексы правых соседей сдвигаются на единицу.
    */
    Node* ContainerNode::insertChild(std::size_t index, std::unique_ptr<Node>&& child)
    {
        if (index > _children.size())
            throw DomException(DomErrorCode::INDEX_OUT_OF_RANGE,
                "индекс вставки " + std::to_string(index) + " больше числа детей");
        checkInsertion(child.get(), index);
        Node* inserted = child.get();
        auto position = _children.begin() + static_cast<std::ptrdiff_t>(index);
        _children.insert(position, std::move(child));
        inserted->setParent(this);
        reindexFrom(index);
        return inserted;
    }

    /*
    ** Отсоединение ребёнка. Владение передаётся вызывающему коду, а у
    ** узла обнуляется ссылка на родителя — он становится полностью
    ** автономным и может быть вставлен в любое другое дерево.
    */
    std::unique_ptr<Node> ContainerNode::removeChild(std::size_t index)
    {
        if (index >= _children.size())
            throw DomException(DomErrorCode::INDEX_OUT_OF_RANGE,
                "нет ребёнка с индексом " + std::to_string(index));
        auto position = _children.begin() + static_cast<std::ptrdiff_t>(index);
        std::unique_ptr<Node> detached = std::move(*position);
        _children.erase(position);
        detached->setParent(nullptr);
        detached->setSiblingIndex(0);
        reindexFrom(index);
        return detached;
    }

    // Очистка: unique_ptr сами освобождают память всех поддеревьев
    void ContainerNode::clearChildren() noexcept
    {
        _children.clear();
    }
}
