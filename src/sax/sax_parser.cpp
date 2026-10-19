#include "sax/sax_parser.hpp"
#include "io/file_io.hpp"
#include "io/mapped_file.hpp"
#include "parser/internal/parse_pipeline.hpp"

#include <istream>
#include <utility>

namespace beresta
{
    // Настройки — единственное состояние парсера
    // The settings are the only state of the parser
    SaxParser::SaxParser(ParserOptions options)
        : _options(std::move(options))
    {
    }

    // Разбор из памяти
    // Parsing from memory
    void SaxParser::parse(std::string_view input, ContentHandler& handler) const
    {
        detail::runSaxParse(input, _options, handler);
    }

    /*
    ** Разбор файла с базовым URI по пути к нему. Файл отображается в
    ** память, а не читается в буфер: собственная память процесса при SAX
    ** (и при проверке по схеме без дерева) остаётся O(глубины документа)
    ** даже для файлов в гигабайты — прочитанные страницы ОС выгружает сама.
    **
    ** Parsing a file with the base URI set to its path. The file is mapped into
    ** memory, not read into a buffer: the process's own memory with SAX
    ** (and with schema validation without a tree) stays O(document depth)
    ** even for files of gigabytes — the OS drops the read pages by itself.
    */
    void SaxParser::parseFile(const std::filesystem::path& path, ContentHandler& handler) const
    {
        io::MappedFile file(path);

        detail::runSaxParse(file.getBytes(), detail::optionsForFile(_options, path), handler);
    }

    // Разбор потока
    // Parsing a stream
    void SaxParser::parse(std::istream& stream, ContentHandler& handler) const
    {
        std::string bytes = io::readStream(stream);

        detail::runSaxParse(bytes, _options, handler);
    }

    // Текущие настройки
    // The current settings
    const ParserOptions& SaxParser::getOptions() const noexcept
    {
        return _options;
    }
}
