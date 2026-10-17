#include "support/recording_handler.hpp"

namespace support
{
    // Закрытие элемента
    void RecordingHandler::endElement(const beresta::SaxElement& element)
    {
        record("end:" + std::string(element.name));
    }

    // Текст
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
    void RecordingHandler::ignorableWhitespace(std::string_view text)
    {
        (void)text;
        record("ws");
    }

    // Комментарий
    void RecordingHandler::comment(std::string_view text)
    {
        record("comment:" + std::string(text));
    }
}
