#include "core/document.hpp"

#include <utility>

namespace beresta
{
    // Изменяемая версия переиспользует константную (DRY)
    Element* Document::getRoot()
    {
        const Document& self = *this;

        return const_cast<Element*>(self.getRoot());
    }

    // Корень — единственный ребёнок типа ELEMENT
    const Element* Document::getRoot() const
    {
        std::size_t index = findFirstOfType(NodeType::ELEMENT);

        if (index == npos)
            return nullptr;
        return static_cast<const Element*>(getChild(index));
    }

    /*
    ** Замена корня на том же месте, чтобы не нарушить порядок пролога
    ** и эпилога (комментариев вокруг). Если корня не было — добавляем в конец.
    */
    Element* Document::setRoot(std::unique_ptr<Element> root)
    {
        std::size_t index = findFirstOfType(NodeType::ELEMENT);
        Element* raw = root.get();
        std::unique_ptr<Node> node = std::move(root);

        if (index == npos)
            appendChild(std::move(node));
        else
            replaceChild(index, std::move(node));
        return raw;
    }

    // Извлечение корня: узел становится автономным, документ — без корня
    std::unique_ptr<Element> Document::releaseRoot()
    {
        std::size_t index = findFirstOfType(NodeType::ELEMENT);

        if (index == npos)
            return nullptr;
        std::unique_ptr<Node> node = removeChild(index);
        return std::unique_ptr<Element>(static_cast<Element*>(node.release()));
    }

    // Документы равны, если совпадают декларации и все дети попарно
    bool Document::isEqualNode(const Node& other) const
    {
        if (other.getType() != NodeType::DOCUMENT)
            return false;
        const Document& document = static_cast<const Document&>(other);
        if (document._declaration != _declaration)
            return false;
        return childrenEqual(document);
    }
}
