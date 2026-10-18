#ifndef SAX_PARSER_HPP
#define SAX_PARSER_HPP

#include "parser/parser_options.hpp"
#include "sax/content_handler.hpp"

#include <filesystem>
#include <iosfwd>
#include <string_view>

namespace beresta
{
    /*
    ** SaxParser — потоковый парсер (аналог SAX2XMLReader в Xerces).
    ** Дерево не строится: обработчик получает события по мере чтения,
    ** поэтому расход памяти не зависит от размера документа.
    ** Все проверки корректности, пространств имён и DTD — те же, что в DOM.
    ** Со схемой (ParserOptions::schema) события проходят через потоковый
    ** валидатор (xsd::ValidatingHandler) к вашему обработчику: проверка без
    ** дерева, память — O(глубины документа). Ошибки схемы приходят в
    ** errorHandler после endDocument (без него — исключение с первой ошибкой).
    **
    ** SaxParser — a streaming parser (the analog of SAX2XMLReader in Xerces).
    ** No tree is built: the handler receives events as the input is read,
    ** so the memory use does not depend on the document size.
    ** All well-formedness, namespace and DTD checks are the same as in DOM.
    ** With a schema (ParserOptions::schema) the events pass through the streaming
    ** validator (xsd::ValidatingHandler) to your handler: validation without a
    ** tree, the memory is O(document depth). Schema errors arrive in
    ** errorHandler after endDocument (without it — an exception with the first error).
    */
    class SaxParser
    {
    public:
        explicit SaxParser(ParserOptions options = ParserOptions());

        // Разбирает документ из памяти, отправляя события в handler
        // Parses a document from memory, sending events to handler
        void parse(std::string_view input, ContentHandler& handler) const;

        // Разбирает файл
        // Parses a file
        void parseFile(const std::filesystem::path& path, ContentHandler& handler) const;

        // Разбирает поток до конца
        // Parses a stream to the end
        void parse(std::istream& stream, ContentHandler& handler) const;

        // Текущие настройки
        // The current settings
        const ParserOptions& getOptions() const noexcept;

    private:
        ParserOptions _options;
    };
}

#endif
