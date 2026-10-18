#include "core/container_node.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Глубокое копирование без рекурсии. Рекурсивный вариант (clone() у
    ** каждого ребёнка вызывает clone() у своих детей) падал бы с
    ** переполнением стека на деревьях глубиной в десятки тысяч уровней.
    **
    ** Здесь каждый контейнер сначала копируется ПУСТЫМ (cloneEmpty), а пара
    ** «оригинал — пустая копия» откладывается в явный стек. Затем стек
    ** разбирается: дети оригинала копируются в копию тем же способом.
    ** Глубина дерева влияет только на размер вектора pending в куче.
    **
    ** При исключении (нехватка памяти) уже построенная часть result
    ** уничтожается его деструктором — утечек нет, оригинал не меняется.
    **
    ** Deep copying without recursion. A recursive variant (clone() of
    ** each child calls clone() of its children) would crash with a
    ** stack overflow on trees tens of thousands of levels deep.
    ** Here each container is first copied EMPTY (cloneEmpty), and the pair
    ** "original — empty copy" is put aside on an explicit stack. Then the stack
    ** is processed: the children of the original are copied into the copy the same way.
    ** The tree depth affects only the size of the pending vector in the heap.
    ** On an exception (out of memory) the already built part of result
    ** is destroyed by its destructor — no leaks, the original does not change.
    */
    ContainerNode::ChildList ContainerNode::cloneChildren(const ContainerNode& source)
    {
        ChildList result;
        std::vector<CopyTask> pending;

        appendCopies(source, result, pending);
        while (!pending.empty())
        {
            CopyTask task = pending.back();
            pending.pop_back();
            appendCopies(*task.first, task.second->_children, pending);
            task.second->adoptChildren();
        }
        return result;
    }

    /*
    ** Один уровень копирования. Листья (текст, комментарий, PI, DOCTYPE)
    ** копируются целиком через clone() — у них нет детей, поэтому это не
    ** рекурсия. Контейнер копируется пустым, его дети — позже, из pending.
    ** reserve до цикла: последующие push_back уже не выделяют память и не
    ** бросают, поэтому указатель в pending не может «повиснуть».
    **
    ** One level of copying. Leaves (text, comment, PI, DOCTYPE)
    ** are copied whole through clone() — they have no children, so this is not
    ** recursion. A container is copied empty, its children later, from pending.
    ** reserve before the loop: the following push_backs no longer allocate memory and
    ** do not throw, so the pointer in pending cannot "dangle".
    */
    void ContainerNode::appendCopies(const ContainerNode& source, ChildList& target,
        std::vector<CopyTask>& pending)
    {
        target.reserve(target.size() + source._children.size());
        for (const std::unique_ptr<Node>& child : source._children)
        {
            const ContainerNode* container = asContainer(*child);
            if (container == nullptr)
            {
                target.push_back(child->clone());
                continue;
            }
            std::unique_ptr<ContainerNode> copy = container->cloneEmpty();
            pending.emplace_back(container, copy.get());
            target.push_back(std::move(copy));
        }
    }

    // Контейнерами в DOM «Берёсты» являются ровно элемент и документ
    // The containers in the Beresta DOM are exactly the element and the document
    const ContainerNode* ContainerNode::asContainer(const Node& node) noexcept
    {
        NodeType type = node.getType();

        if (type != NodeType::ELEMENT && type != NodeType::DOCUMENT)
            return nullptr;
        return static_cast<const ContainerNode*>(&node);
    }

    // Изменяемая версия переиспользует константную (DRY)
    // The mutable version reuses the const one (DRY)
    ContainerNode* ContainerNode::asContainer(Node& node) noexcept
    {
        const Node& constant = node;

        return const_cast<ContainerNode*>(asContainer(constant));
    }
}
