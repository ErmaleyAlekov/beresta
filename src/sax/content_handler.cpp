#include "sax/content_handler.hpp"

namespace beresta
{
    // Виртуальный деструктор интерфейса
    // The virtual destructor of the interface
    ContentHandler::~ContentHandler()
    {
    }

    // По умолчанию позиция не нужна
    // By default the position is not needed
    void ContentHandler::setDocumentLocator(const Locator& locator)
    {
        (void)locator;
    }

    // Начало документа — по умолчанию ничего не делаем
    // The start of the document — does nothing by default
    void ContentHandler::startDocument()
    {
    }

    // Конец документа — по умолчанию ничего не делаем
    // The end of the document — does nothing by default
    void ContentHandler::endDocument()
    {
    }

    // Декларация — по умолчанию игнорируется
    // The declaration — ignored by default
    void ContentHandler::xmlDeclaration(const XmlDeclaration& declaration)
    {
        (void)declaration;
    }
}
