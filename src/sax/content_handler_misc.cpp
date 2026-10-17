#include "sax/content_handler.hpp"
#include "sax/locator.hpp"

namespace beresta
{
    // Виртуальный деструктор интерфейса Locator
    Locator::~Locator()
    {
    }

    // Пробельное форматирование — по умолчанию игнорируется
    void ContentHandler::ignorableWhitespace(std::string_view text)
    {
        (void)text;
    }

    // Комментарий — по умолчанию игнорируется
    void ContentHandler::comment(std::string_view text)
    {
        (void)text;
    }

    // Инструкция обработки — по умолчанию игнорируется
    void ContentHandler::processingInstruction(std::string_view target, std::string_view data)
    {
        (void)target;
        (void)data;
    }
}
