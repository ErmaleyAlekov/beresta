#include "core/document.hpp"

namespace beresta
{
    /*
    ** Крючки обхода ContainerNode для документа (см. element_hooks.cpp).
    ** Документ никогда не бывает ребёнком, поэтому serializeOpen и
    ** serializeClose вызываются только из Document::serialize.
    **
    ** The ContainerNode traversal hooks for the document (see element_hooks.cpp).
    ** A document is never a child, so serializeOpen and
    ** serializeClose are called only from Document::serialize.
    */

    // Пустой документ с той же XML-декларацией и позицией в исходнике
    // An empty document with the same XML declaration and source position
    std::unique_ptr<ContainerNode> Document::cloneEmpty() const
    {
        std::unique_ptr<Document> copy = std::make_unique<Document>();

        copy->_declaration = _declaration;
        copy->setSourceLocation(getSourceLocation());
        return copy;
    }

    // Собственное свойство документа — XML-декларация
    // The document's own property — the XML declaration
    bool Document::equalsShallow(const ContainerNode& other) const
    {
        return static_cast<const Document&>(other)._declaration == _declaration;
    }

    // Начало документа — XML-декларация, если она есть
    // The start of the document — the XML declaration, if there is one
    void Document::serializeOpen(std::string& output) const
    {
        if (_declaration.has_value())
            serializeXmlDeclaration(_declaration.value(), output);
    }

    // У документа нет закрывающей разметки
    // A document has no closing markup
    void Document::serializeClose(std::string& output) const
    {
        (void)output;
    }
}
