#include "serializer/xml_serializer.hpp"
#include "io/file_io.hpp"

#include <ostream>
#include <utility>

namespace beresta
{
    // Настройки — единственное состояние сериализатора
    // The settings are the only state of the serializer
    XmlSerializer::XmlSerializer(SerializerOptions options)
        : _options(std::move(options))
    {
    }

    // Сначала текст в UTF-8 с нужной разметкой, затем перекодирование
    // First the text in UTF-8 with the required markup, then the transcoding
    std::string XmlSerializer::toString(const Node& node) const
    {
        return encode(serializeUtf8(node));
    }

    // Запись в поток с проверкой ошибок ввода-вывода
    // Writing to a stream with an I/O error check
    void XmlSerializer::write(const Node& node, std::ostream& stream) const
    {
        io::writeStream(stream, toString(node));
    }

    // Запись в файл (двоичный режим: переводы строк не искажаются ОС)
    // Writing to a file (binary mode: the OS does not distort line breaks)
    void XmlSerializer::writeFile(const Node& node, const std::filesystem::path& path) const
    {
        io::writeFile(path, toString(node));
    }

    // Текущие настройки
    // The current settings
    const SerializerOptions& XmlSerializer::getOptions() const noexcept
    {
        return _options;
    }
}
