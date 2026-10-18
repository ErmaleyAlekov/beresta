#include "core/container_node.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Разбор дерева без рекурсии и без выделения памяти. Обычный деструктор
    ** вектора unique_ptr уничтожал бы ребёнка, тот — своих детей и т.д.:
    ** глубина рекурсии равна глубине дерева, и глубокое дерево переполнило
    ** бы стек. Здесь спуск идёт к самому глубокому последнему потомку, он
    ** уничтожается (у него уже нет детей), а подъём — по указателю на
    ** родителя. Каждый узел посещается O(1) раз, деструктор не бросает.
    **
    ** Takes the tree apart without recursion and without memory allocation. An ordinary
    ** destructor of a vector of unique_ptr would destroy a child, the child its children
    ** and so on: the recursion depth equals the tree depth, and a deep tree would
    ** overflow the stack. Here the descent goes to the deepest last descendant, it
    ** is destroyed (it has no children any more), and the ascent goes through the parent
    ** pointer. Each node is visited O(1) times, the destructor does not throw.
    */
    ContainerNode::~ContainerNode()
    {
        ContainerNode* current = this;

        for (;;)
        {
            if (current->_children.empty())
            {
                if (current == this)
                    break;
                current = current->getParent();
                continue;
            }
            ContainerNode* last = asContainer(*current->_children.back());
            if (last != nullptr && !last->_children.empty())
                current = last;
            else
                current->_children.pop_back();
        }
    }

    // Вставка в конец — частный случай вставки по индексу
    // Appending to the end is a special case of inserting by index
    Node* ContainerNode::appendChild(std::unique_ptr<Node>&& child)
    {
        return insertChild(_children.size(), std::move(child));
    }

    /*
    ** Вставка по индексу. Сначала все проверки (индекс, структура, тип),
    ** и только потом изменение состояния — при ошибке дерево не меняется.
    ** После вставки индексы правых соседей сдвигаются на единицу.
    **
    ** Inserting by index. First all checks (the index, the structure, the type),
    ** and only then the state change — on error the tree does not change.
    ** After the insertion the indexes of the right siblings shift by one.
    */
    Node* ContainerNode::insertChild(std::size_t index, std::unique_ptr<Node>&& child)
    {
        if (index > _children.size())
            throw DomException(DomErrorCode::INDEX_OUT_OF_RANGE,
                i18n::format(i18n::Message::INSERT_INDEX_TOO_LARGE, {std::to_string(index)}));
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
    **
    ** Detaching a child. The ownership is passed to the caller, and the
    ** node's parent link is reset — it becomes fully standalone
    ** and can be inserted into any other tree.
    */
    std::unique_ptr<Node> ContainerNode::removeChild(std::size_t index)
    {
        if (index >= _children.size())
            throw DomException(DomErrorCode::INDEX_OUT_OF_RANGE,
                i18n::format(i18n::Message::NO_CHILD_AT_INDEX, {std::to_string(index)}));
        auto position = _children.begin() + static_cast<std::ptrdiff_t>(index);
        std::unique_ptr<Node> detached = std::move(*position);
        _children.erase(position);
        detached->setParent(nullptr);
        detached->setSiblingIndex(0);
        reindexFrom(index);
        return detached;
    }

    // Очистка: unique_ptr сами освобождают память всех поддеревьев
    // Clearing: the unique_ptrs free the memory of all subtrees themselves
    void ContainerNode::clearChildren() noexcept
    {
        _children.clear();
    }
}
