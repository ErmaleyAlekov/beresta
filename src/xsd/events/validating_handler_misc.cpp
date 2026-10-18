#include "xsd/validating_handler.hpp"
#include "xsd/internal/stream_validator.hpp"

namespace beresta::xsd
{
    // Комментарий — ребёнок для xsi:nil (если DomBuilder его не выбрасывает)
    // A comment is a child for xsi:nil (unless DomBuilder discards it)
    void ValidatingHandler::comment(std::string_view text)
    {
        _validator->otherChild(true);
        if (_downstream != nullptr)
            _downstream->comment(text);
    }

    // Инструкция обработки — тоже ребёнок для xsi:nil
    // A processing instruction is a child for xsi:nil too
    void ValidatingHandler::processingInstruction(std::string_view target, std::string_view data)
    {
        _validator->otherChild(false);
        if (_downstream != nullptr)
            _downstream->processingInstruction(target, data);
    }

    // PI из DTD к содержимому элементов не относятся — только дальше
    // PIs from the DTD do not belong to element content — only passed on
    void ValidatingHandler::dtdProcessingInstruction(std::string_view target, std::string_view data)
    {
        if (_downstream != nullptr)
            _downstream->dtdProcessingInstruction(target, data);
    }
}
