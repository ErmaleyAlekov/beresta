#include "core/node.hpp"
#include "core/container_node.hpp"
#include "core/document.hpp"

namespace beresta
{
    // Изменяемая версия переиспользует константную (DRY)
    Document* Node::getOwnerDocument()
    {
        const Node& self = *this;

        return const_cast<Document*>(self.getOwnerDocument());
    }

    /*
    ** Поднимаемся к самому верхнему предку. Если это документ — он и есть
    ** владелец. Автономное поддерево (корень — элемент) документа не имеет.
    */
    const Document* Node::getOwnerDocument() const
    {
        const Node* top = this;

        while (top->getParent() != nullptr)
            top = top->getParent();
        if (top->getType() != NodeType::DOCUMENT)
            return nullptr;
        return static_cast<const Document*>(top);
    }

    // Позиция узла в исходном тексте
    const SourceLocation& Node::getSourceLocation() const noexcept
    {
        return _location;
    }

    // Позицию записывает парсер; пользователь тоже может задать её вручную
    void Node::setSourceLocation(SourceLocation location) noexcept
    {
        _location = location;
    }
}
