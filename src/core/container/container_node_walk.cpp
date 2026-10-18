#include "core/container_node.hpp"
#include "core/character_node.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Обходы поддерева без рекурсии. Каждый использует явный стек «контейнер
    ** + позиция следующего ребёнка» в куче: глубина дерева ограничена только
    ** памятью, а не стеком вызовов (около 1 МБ на Windows и 8 МБ на Linux).
    **
    ** Subtree traversals without recursion. Each uses an explicit stack "container
    ** + position of the next child" in the heap: the tree depth is limited only by
    ** memory, not by the call stack (about 1 MB on Windows and 8 MB on Linux).
    */
    using WalkStack = std::vector<std::pair<const ContainerNode*, std::size_t>>;

    /*
    ** Сериализация детей. Лист и пустой контейнер выводятся целиком своим
    ** serialize() (там нет детей — значит, нет и рекурсии); у непустого
    ** контейнера выводится начало разметки, а конец — когда разобраны все
    ** его дети. Сам this открывает и закрывает вызывающий код.
    **
    ** Serializing the children. A leaf and an empty container are written whole by their
    ** serialize() (there are no children there — so there is no recursion); for a non-empty
    ** container the start of the markup is written, and the end — when all its
    ** children are processed. The caller opens and closes this itself.
    */
    void ContainerNode::serializeChildren(std::string& output) const
    {
        WalkStack stack{{this, 0}};

        while (!stack.empty())
        {
            const ContainerNode* current = stack.back().first;
            std::size_t index = stack.back().second++;
            if (index == current->_children.size())
            {
                if (current != this)
                    current->serializeClose(output);
                stack.pop_back();
                continue;
            }
            const Node& child = *current->_children[index];
            const ContainerNode* container = asContainer(child);
            if (container == nullptr || container->_children.empty())
                child.serialize(output);
            else
            {
                container->serializeOpen(output);
                stack.emplace_back(container, 0);
            }
        }
    }

    /*
    ** Сбор текста: TEXT и CDATA дают свои значения, вложенные контейнеры
    ** обходятся по стеку, а комментарии и PI пропускаются (как в W3C DOM).
    **
    ** Collecting text: TEXT and CDATA give their values, nested containers
    ** are traversed through the stack, and comments and PIs are skipped (as in W3C DOM).
    */
    void ContainerNode::collectText(const ContainerNode& container, std::string& output)
    {
        WalkStack stack{{&container, 0}};

        while (!stack.empty())
        {
            const ContainerNode* current = stack.back().first;
            std::size_t index = stack.back().second++;
            if (index == current->_children.size())
            {
                stack.pop_back();
                continue;
            }
            const Node& child = *current->_children[index];
            NodeType type = child.getType();
            if (type == NodeType::TEXT || type == NodeType::CDATA_SECTION)
                output += static_cast<const CharacterNode&>(child).getValue();
            else if (asContainer(child) != nullptr)
                stack.emplace_back(asContainer(child), 0);
        }
    }

    /*
    ** Сравнение поддеревьев. Пары контейнеров откладываются в стек; для
    ** каждой пары сравниваются число детей, затем дети попарно: листья —
    ** своим isEqualNode, контейнеры — собственными свойствами (equalsShallow),
    ** а их дети — позже, когда пара дойдёт до вершины стека.
    **
    ** Comparing subtrees. Pairs of containers are put aside on a stack; for
    ** each pair the number of children is compared, then the children pairwise: leaves —
    ** by their isEqualNode, containers — by their own properties (equalsShallow),
    ** and their children — later, when the pair reaches the top of the stack.
    */
    bool ContainerNode::childrenEqual(const ContainerNode& other) const
    {
        std::vector<std::pair<const ContainerNode*, const ContainerNode*>> pending{{this, &other}};

        while (!pending.empty())
        {
            std::pair<const ContainerNode*, const ContainerNode*> pair = pending.back();
            pending.pop_back();
            if (pair.first->_children.size() != pair.second->_children.size())
                return false;
            for (std::size_t index = 0; index < pair.first->_children.size(); ++index)
            {
                const Node& left = *pair.first->_children[index];
                const Node& right = *pair.second->_children[index];
                const ContainerNode* container = asContainer(left);
                if (container == nullptr && !left.isEqualNode(right))
                    return false;
                if (container == nullptr)
                    continue;
                if (left.getType() != right.getType() || !container->equalsShallow(*asContainer(right)))
                    return false;
                pending.emplace_back(container, asContainer(right));
            }
        }
        return true;
    }
}
