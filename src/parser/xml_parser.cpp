#include "parser/xml_parser.hpp"
#include "io/file_io.hpp"
#include "parser/internal/parse_pipeline.hpp"

#include <istream>

namespace beresta
{
    // Сохраняем настройки; никакого другого состояния у парсера нет
    XmlParser::XmlParser(ParserOptions options)
        : _options(std::move(options))
    {
    }

    // Разбор из памяти
    Document XmlParser::parse(std::string_view input) const
    {
        return parseWithOptions(input, _options);
    }

    // Файл читается целиком в двоичном режиме — кодировку определит конвейер
    Document XmlParser::parseFile(const std::filesystem::path& path) const
    {
        std::string bytes = io::readFile(path);

        return parseWithOptions(bytes, detail::optionsForFile(_options, path));
    }

    // Поток читается до конца, затем разбирается как обычный буфер
    Document XmlParser::parse(std::istream& stream) const
    {
        std::string bytes = io::readStream(stream);

        return parseWithOptions(bytes, _options);
    }

    // Фрагмент — это документ, из которого извлекается автономный корень
    std::unique_ptr<Element> XmlParser::parseElement(std::string_view input) const
    {
        Document document = parse(input);

        return document.releaseRoot();
    }
}
