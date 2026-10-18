#include "core/node.hpp"

namespace beresta
{
    // Новый узел создаётся автономным — без родителя
    // A new node is created standalone — without a parent
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
    **
    ** Copying a node. The parent is deliberately NOT copied:
    ** the copy is a new independent node that is not inserted anywhere yet.
    ** Otherwise two entities would consider themselves children of one parent,
    ** although it owns only one of them. The source position
    ** is copied: it is information about the origin of the data, not about the place in the tree.
    */
    Node::Node(const Node& other) noexcept
        : _parent(nullptr), _siblingIndex(0), _location(other._location)
    {
    }

    // Перемещённый узел также становится автономным (по той же причине)
    // A moved node becomes standalone too (for the same reason)
    Node::Node(Node&& other) noexcept
        : _parent(nullptr), _siblingIndex(0), _location(other._location)
    {
    }

    /*
    ** Присваивание меняет СОДЕРЖИМОЕ узла, но не его место в дереве:
    ** собственные _parent и _siblingIndex сохраняются, чужие — игнорируются.
    **
    ** Assignment changes the CONTENT of a node, but not its place in the tree:
    ** its own _parent and _siblingIndex are kept, the other's are ignored.
    */
    Node& Node::operator=(const Node& other) noexcept
    {
        _location = other._location;
        return *this;
    }

    // Перемещающее присваивание: аналогично, позиция в дереве не меняется
    // Move assignment: likewise, the position in the tree does not change
    Node& Node::operator=(Node&& other) noexcept
    {
        _location = other._location;
        return *this;
    }
}
