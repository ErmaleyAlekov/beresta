#include "parser/dom_builder.hpp"
#include "core/comment.hpp"
#include "core/processing_instruction.hpp"

namespace beresta
{
    // Декларация — свойство документа, а не узел
    void DomBuilder::xmlDeclaration(const XmlDeclaration& declaration)
    {
        _document.setDeclaration(declaration);
    }

    // DOCTYPE копируется вместе с разделяемой моделью DTD
    void DomBuilder::documentType(const DocumentType& documentType)
    {
        attach(std::make_unique<DocumentType>(documentType));
    }

    // Комментарий (если их не просили отбрасывать)
    void DomBuilder::comment(std::string_view text)
    {
        if (_discardComments)
            return;
        attach(std::make_unique<Comment>(std::string(text)));
    }

    // Инструкция обработки
    void DomBuilder::processingInstruction(std::string_view target, std::string_view data)
    {
        attach(std::make_unique<ProcessingInstruction>(std::string(target), std::string(data)));
    }
}
