#include "core/node.hpp"

namespace beresta
{
    // Новый узел создаётся автономным — без родителя
    Node::Node() noexcept
        : _parent(nullptr), _siblingIndex(0), _location()
    {
    }

    /*
    ** Копирование узла. Родитель НЕ копируется намеренно:
    ** копия — новый самостоятельный узел, который ещё никуда не вставлен.
    ** Иначе две сущности считали бы себя детьми одного родителя,
    ** хотя тот владеет лишь одной из них. Позиция в исходнике
    ** копируется: это сведения о происхождении данных, а не о месте в дереве.
    */
    Node::Node(const Node& other) noexcept
        : _parent(nullptr), _siblingIndex(0), _location(other._location)
    {
    }

    // Перемещённый узел также становится автономным (по той же причине)
    Node::Node(Node&& other) noexcept
        : _parent(nullptr), _siblingIndex(0), _location(other._location)
    {
    }

    /*
    ** Присваивание меняет СОДЕРЖИМОЕ узла, но не его место в дереве:
    ** собственные _parent и _siblingIndex сохраняются, чужие — игнорируются.
    */
    Node& Node::operator=(const Node& other) noexcept
    {
        _location = other._location;
        return *this;
    }

    // Перемещающее присваивание: аналогично, позиция в дереве не меняется
    Node& Node::operator=(Node&& other) noexcept
    {
        _location = other._location;
        return *this;
    }
}
