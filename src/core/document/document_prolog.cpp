#include "core/document.hpp"

#include <utility>

namespace beresta
{
    // Текущая декларация
    const std::optional<XmlDeclaration>& Document::getDeclaration() const noexcept
    {
        return _declaration;
    }

    // Новая декларация проходит проверку формата; nullopt удаляет декларацию
    void Document::setDeclaration(std::optional<XmlDeclaration> declaration)
    {
        if (declaration.has_value())
            validateXmlDeclaration(declaration.value());
        _declaration = std::move(declaration);
    }

    // Изменяемая версия переиспользует константную (DRY)
    DocumentType* Document::getDocumentType()
    {
        const Document& self = *this;

        return const_cast<DocumentType*>(self.getDocumentType());
    }

    // DOCTYPE — единственный ребёнок типа DOCUMENT_TYPE
    const DocumentType* Document::getDocumentType() const
    {
        std::size_t index = findFirstOfType(NodeType::DOCUMENT_TYPE);

        if (index == npos)
            return nullptr;
        return static_cast<const DocumentType*>(getChild(index));
    }
}
