#include "xsd/validating_handler.hpp"
#include "xsd/internal/stream_validator.hpp"

namespace beresta::xsd
{
    // Позиции событий нужны и валидатору (для ошибок), и следующему обработчику
    // Event positions are needed both by the validator (for errors) and by the next handler
    void ValidatingHandler::setDocumentLocator(const Locator& locator)
    {
        _validator->setLocator(&locator);
        if (_downstream != nullptr)
            _downstream->setDocumentLocator(locator);
    }

    // Начало документа проверке не нужно — только передаётся дальше
    // The document start is not needed by the validation — it is only passed on
    void ValidatingHandler::startDocument()
    {
        if (_downstream != nullptr)
            _downstream->startDocument();
    }

    // Конец документа: висячие IDREF и ключи, затем следующий обработчик
    // The end of the document: dangling IDREFs and keys, then the next handler
    void ValidatingHandler::endDocument()
    {
        _validator->endDocument();
        if (_downstream != nullptr)
            _downstream->endDocument();
    }

    // XML-декларация — только дальше
    // The XML declaration — only passed on
    void ValidatingHandler::xmlDeclaration(const XmlDeclaration& declaration)
    {
        if (_downstream != nullptr)
            _downstream->xmlDeclaration(declaration);
    }

    // DOCTYPE — только дальше (схема не зависит от DTD)
    // DOCTYPE — only passed on (the schema does not depend on the DTD)
    void ValidatingHandler::documentType(const DocumentType& documentType)
    {
        if (_downstream != nullptr)
            _downstream->documentType(documentType);
    }
}
