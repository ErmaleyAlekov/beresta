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
    */
    class SaxParser
    {
    public:
        explicit SaxParser(ParserOptions options = ParserOptions());

        // Разбирает документ из памяти, отправляя события в handler
        void parse(std::string_view input, ContentHandler& handler) const;

        // Разбирает файл
        void parseFile(const std::filesystem::path& path, ContentHandler& handler) const;

        // Разбирает поток до конца
        void parse(std::istream& stream, ContentHandler& handler) const;

        // Текущие настройки
        const ParserOptions& getOptions() const noexcept;

    private:
        ParserOptions _options;
    };
}

#endif
