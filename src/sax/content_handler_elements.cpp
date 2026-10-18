#include "sax/content_handler.hpp"

namespace beresta
{
    // DOCTYPE — по умолчанию игнорируется
    // The DOCTYPE — ignored by default
    void ContentHandler::documentType(const DocumentType& documentType)
    {
        (void)documentType;
    }

    // Открытие элемента — по умолчанию игнорируется
    // The start of an element — ignored by default
    void ContentHandler::startElement(const SaxElement& element)
    {
        (void)element;
    }

    // Закрытие элемента — по умолчанию игнорируется
    // The end of an element — ignored by default
    void ContentHandler::endElement(const SaxElement& element)
    {
        (void)element;
    }

    // Текст — по умолчанию игнорируется
    // Text — ignored by default
    void ContentHandler::characters(std::string_view text)
    {
        (void)text;
    }

    // CDATA — по умолчанию игнорируется
    // CDATA — ignored by default
    void ContentHandler::cdata(std::string_view text)
    {
        (void)text;
    }
}
