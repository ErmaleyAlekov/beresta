#include "sax/sax_parser.hpp"
#include "io/file_io.hpp"
#include "parser/internal/parse_pipeline.hpp"

#include <istream>
#include <utility>

namespace beresta
{
    // Настройки — единственное состояние парсера
    SaxParser::SaxParser(ParserOptions options)
        : _options(std::move(options))
    {
    }

    // Разбор из памяти
    void SaxParser::parse(std::string_view input, ContentHandler& handler) const
    {
        detail::runParse(input, _options, handler);
    }

    // Разбор файла с базовым URI по пути к нему
    void SaxParser::parseFile(const std::filesystem::path& path, ContentHandler& handler) const
    {
        std::string bytes = io::readFile(path);

        detail::runParse(bytes, detail::optionsForFile(_options, path), handler);
    }

    // Разбор потока
    void SaxParser::parse(std::istream& stream, ContentHandler& handler) const
    {
        std::string bytes = io::readStream(stream);

        detail::runParse(bytes, _options, handler);
    }

    // Текущие настройки
    const ParserOptions& SaxParser::getOptions() const noexcept
    {
        return _options;
    }
}
