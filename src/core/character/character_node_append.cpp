#include "core/character_node.hpp"

namespace beresta
{
    /*
    ** Дописывание без копирования всего значения: раньше сборщик дерева
    ** склеивал previous.getValue() + text на каждом куске текста, и тысячи
    ** подряд идущих секций CDATA (coalesceCData) или ссылок на сущности
    ** давали квадратичное время. Проверяется только «окно» на стыке.
    **
    ** Appending without copying the whole value: the tree builder used to
    ** concatenate previous.getValue() + text on every piece of text, and thousands
    ** of consecutive CDATA sections (coalesceCData) or entity references
    ** gave quadratic time. Only the "window" at the joint is checked.
    */
    void CharacterNode::appendValue(std::string_view piece)
    {
        std::size_t kept = 2;

        if (_value.size() < kept)
            kept = _value.size();
        std::string joint = _value.substr(_value.size() - kept);
        joint.append(piece);
        validateValue(joint);
        _value.append(piece);
    }
}
