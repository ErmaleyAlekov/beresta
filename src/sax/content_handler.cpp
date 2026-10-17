#include "sax/content_handler.hpp"

namespace beresta
{
    // Виртуальный деструктор интерфейса
    ContentHandler::~ContentHandler()
    {
    }

    // По умолчанию позиция не нужна
    void ContentHandler::setDocumentLocator(const Locator& locator)
    {
        (void)locator;
    }

    // Начало документа — по умолчанию ничего не делаем
    void ContentHandler::startDocument()
    {
    }

    // Конец документа — по умолчанию ничего не делаем
    void ContentHandler::endDocument()
    {
    }

    // Декларация — по умолчанию игнорируется
    void ContentHandler::xmlDeclaration(const XmlDeclaration& declaration)
    {
        (void)declaration;
    }
}
