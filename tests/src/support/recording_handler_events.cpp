#include "support/recording_handler.hpp"

namespace support
{
    // Закрытие элемента
    // The end of an element
    void RecordingHandler::endElement(const beresta::SaxElement& element)
    {
        record("end:" + std::string(element.name));
    }

    // Текст
    // Text
    void RecordingHandler::characters(std::string_view text)
    {
        record("text:" + std::string(text));
    }

    // CDATA
    void RecordingHandler::cdata(std::string_view text)
    {
        record("cdata:" + std::string(text));
    }

    // Пробельное форматирование отмечается без содержимого
    // Whitespace formatting is marked without its content
    void RecordingHandler::ignorableWhitespace(std::string_view text)
    {
        (void)text;
        record("ws");
    }

    // Комментарий
    // A comment
    void RecordingHandler::comment(std::string_view text)
    {
        record("comment:" + std::string(text));
    }
}
