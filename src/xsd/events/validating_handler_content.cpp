#include "xsd/validating_handler.hpp"
#include "xsd/internal/stream_validator.hpp"

namespace beresta::xsd
{
    // Сначала проверка, затем следующий обработчик — он видит событие без изменений
    // First the check, then the next handler — it sees the event unchanged
    void ValidatingHandler::startElement(const SaxElement& element)
    {
        _validator->startElement(element);
        if (_downstream != nullptr)
            _downstream->startElement(element);
    }

    // Закрытие: итог значения и модели содержимого
    // Closing: the result of the value and the content model
    void ValidatingHandler::endElement(const SaxElement& element)
    {
        _validator->endElement();
        if (_downstream != nullptr)
            _downstream->endElement(element);
    }

    // Значимый текст
    // Significant text
    void ValidatingHandler::characters(std::string_view text)
    {
        _validator->text(text, false);
        if (_downstream != nullptr)
            _downstream->characters(text);
    }

    // Секция CDATA
    // A CDATA section
    void ValidatingHandler::cdata(std::string_view text)
    {
        _validator->text(text, true);
        if (_downstream != nullptr)
            _downstream->cdata(text);
    }

    // Пробелы форматирования в дерево не попадают — для проверки их тоже нет
    // Formatting whitespace does not get into the tree — for the validation it does not exist either
    void ValidatingHandler::ignorableWhitespace(std::string_view text)
    {
        if (_downstream != nullptr)
            _downstream->ignorableWhitespace(text);
    }
}
