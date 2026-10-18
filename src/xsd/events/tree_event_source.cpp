#include "xsd/internal/tree_event_source.hpp"

namespace beresta::xsd::detail
{
    // Источник ещё не стоит ни на одном узле
    // The source does not stand on any node yet
    TreeEventSource::TreeEventSource()
        : _current(nullptr), _attributes()
    {
    }

    /*
    ** Обход без рекурсии: вход в узел (элемент с детьми — спуск к первому
    ** ребёнку), иначе выход — к соседу или вверх с закрытием предков.
    ** Глубина документа не ограничена размером стека.
    **
    ** A traversal without recursion: entering a node (an element with children — descending to the first
    ** child), otherwise leaving — to the sibling or upwards with closing the ancestors.
    ** The document depth is not limited by the stack size.
    */
    void TreeEventSource::run(const Document& document, ContentHandler& handler)
    {
        const Element* root = document.getRoot();
        const Node* node = root;

        handler.setDocumentLocator(*this);
        handler.startDocument();
        while (node != nullptr)
        {
            if (enter(*node, handler))
                node = static_cast<const Element*>(node)->getFirstChild();
            else
                node = leave(*node, *root, handler);
        }
        handler.endDocument();
    }

    // Позиция текущего узла (у дерева, разобранного без trackLocations, — нулевая)
    // The position of the current node (zero for a tree parsed without trackLocations)
    SourceLocation TreeEventSource::getLocation() const
    {
        if (_current == nullptr)
            return SourceLocation();
        return _current->getSourceLocation();
    }
}
