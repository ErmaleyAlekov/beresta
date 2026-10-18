#include "core/node.hpp"
#include "core/container_node.hpp"

namespace beresta
{
    /*
    ** Сериализация в строку. Вся работа делегируется serialize(),
    ** который дописывает в один общий буфер — так всё дерево
    ** собирается без промежуточных временных строк.
    **
    ** Serialization to a string. All the work is delegated to serialize(),
    ** which appends to one common buffer — so the whole tree is
    ** assembled without intermediate temporary strings.
    */
    std::string Node::toString() const
    {
        std::string output;

        serialize(output);
        return output;
    }

    /*
    ** Поднимаемся от родителя к корню и сравниваем адреса.
    ** Нужен для защиты от циклов: нельзя вставить узел внутрь
    ** его же собственного поддерева.
    **
    ** Climbs from the parent to the root comparing addresses.
    ** Needed to protect against cycles: a node cannot be inserted inside
    ** its own subtree.
    */
    bool Node::isDescendantOf(const Node& ancestor) const noexcept
    {
        const Node* cursor = getParent();

        while (cursor != nullptr)
        {
            if (cursor == &ancestor)
                return true;
            cursor = cursor->getParent();
        }
        return false;
    }
}
