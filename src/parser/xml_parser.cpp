#include "parser/xml_parser.hpp"
#include "io/file_io.hpp"
#include "io/mapped_file.hpp"
#include "parser/internal/parse_pipeline.hpp"

#include <istream>

namespace beresta
{
    // Сохраняем настройки; никакого другого состояния у парсера нет
    // The settings are stored; the parser has no other state
    XmlParser::XmlParser(ParserOptions options)
        : _options(std::move(options))
    {
    }

    // Разбор из памяти
    // Parsing from memory
    Document XmlParser::parse(std::string_view input) const
    {
        return parseWithOptions(input, _options);
    }

    /*
    ** Файл отображается в память (MappedFile), а не копируется: дерево
    ** хранит собственные строки, поэтому байты файла нужны только на время
    ** разбора. Кодировку определит конвейер.
    **
    ** The file is mapped into memory (MappedFile), not copied: the tree
    ** keeps its own strings, so the file bytes are needed only during
    ** parsing. The pipeline will detect the encoding.
    */
    Document XmlParser::parseFile(const std::filesystem::path& path) const
    {
        io::MappedFile file(path);

        return parseWithOptions(file.getBytes(), detail::optionsForFile(_options, path));
    }

    // Поток читается до конца, затем разбирается как обычный буфер
    // The stream is read to the end, then parsed as an ordinary buffer
    Document XmlParser::parse(std::istream& stream) const
    {
        std::string bytes = io::readStream(stream);

        return parseWithOptions(bytes, _options);
    }

    // Фрагмент — это документ, из которого извлекается автономный корень
    // A fragment is a document from which a standalone root is extracted
    std::unique_ptr<Element> XmlParser::parseElement(std::string_view input) const
    {
        Document document = parse(input);

        return document.releaseRoot();
    }
}
