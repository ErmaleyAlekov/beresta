#include "parser/dom_builder.hpp"
#include "core/comment.hpp"
#include "core/processing_instruction.hpp"

namespace beresta
{
    // Декларация — свойство документа, а не узел
    // The declaration is a document property, not a node
    void DomBuilder::xmlDeclaration(const XmlDeclaration& declaration)
    {
        _document.setDeclaration(declaration);
    }

    // DOCTYPE копируется вместе с разделяемой моделью DTD
    // The DOCTYPE is copied together with the shared DTD model
    void DomBuilder::documentType(const DocumentType& documentType)
    {
        attach(std::make_unique<DocumentType>(documentType));
    }

    // Комментарий (если их не просили отбрасывать)
    // A comment (unless discarding them was requested)
    void DomBuilder::comment(std::string_view text)
    {
        if (_discardComments)
            return;
        attach(std::make_unique<Comment>(std::string(text)));
    }

    // Инструкция обработки
    // A processing instruction
    void DomBuilder::processingInstruction(std::string_view target, std::string_view data)
    {
        attach(std::make_unique<ProcessingInstruction>(std::string(target), std::string(data)));
    }

    /*
    ** PI из DTD в дерево не попадает: узел DocumentType не имеет детей, а
    ** вынести PI на верхний уровень значило бы исказить порядок пролога.
    ** Текст PI внутреннего подмножества сохраняется в getInternalSubset().
    **
    ** A PI from the DTD does not go into the tree: a DocumentType node has no children, and
    ** moving the PI to the top level would distort the prolog order.
    ** The text of the internal subset PI is kept in getInternalSubset().
    */
    void DomBuilder::dtdProcessingInstruction(std::string_view target, std::string_view data)
    {
        (void)target;
        (void)data;
    }
}
