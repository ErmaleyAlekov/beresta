#include "core/character_node.hpp"

namespace beresta
{
    /*
    ** Сравнение символьных узлов: типы должны совпадать (Text не равен
    ** Comment с тем же значением), после чего сравниваются значения.
    ** Совпадение типа гарантирует, что other — тоже CharacterNode.
    **
    ** Comparison of character nodes: the types must match (Text is not equal to
    ** a Comment with the same value), then the values are compared.
    ** Matching types guarantee that other is a CharacterNode too.
    */
    bool CharacterNode::isEqualNode(const Node& other) const
    {
        if (other.getType() != getType())
            return false;
        return static_cast<const CharacterNode&>(other).getValue() == _value;
    }
}
