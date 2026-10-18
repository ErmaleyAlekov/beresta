#include "sax/content_handler.hpp"
#include "sax/locator.hpp"

namespace beresta
{
    // Пробельное форматирование — по умолчанию игнорируется
    // Whitespace formatting — ignored by default
    void ContentHandler::ignorableWhitespace(std::string_view text)
    {
        (void)text;
    }

    // Комментарий — по умолчанию игнорируется
    // A comment — ignored by default
    void ContentHandler::comment(std::string_view text)
    {
        (void)text;
    }

    // Инструкция обработки — по умолчанию игнорируется
    // A processing instruction — ignored by default
    void ContentHandler::processingInstruction(std::string_view target, std::string_view data)
    {
        (void)target;
        (void)data;
    }

    // PI из DTD — по умолчанию та же обработка, что и у PI в документе
    // A PI from the DTD — by default the same processing as for a PI in the document
    void ContentHandler::dtdProcessingInstruction(std::string_view target, std::string_view data)
    {
        processingInstruction(target, data);
    }
}
