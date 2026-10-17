#include "sax/content_handler.hpp"

namespace beresta
{
    // DOCTYPE — по умолчанию игнорируется
    void ContentHandler::documentType(const DocumentType& documentType)
    {
        (void)documentType;
    }

    // Открытие элемента — по умолчанию игнорируется
    void ContentHandler::startElement(const SaxElement& element)
    {
        (void)element;
    }

    // Закрытие элемента — по умолчанию игнорируется
    void ContentHandler::endElement(const SaxElement& element)
    {
        (void)element;
    }

    // Текст — по умолчанию игнорируется
    void ContentHandler::characters(std::string_view text)
    {
        (void)text;
    }

    // CDATA — по умолчанию игнорируется
    void ContentHandler::cdata(std::string_view text)
    {
        (void)text;
    }
}
