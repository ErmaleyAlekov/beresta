#ifndef XML_PARSER_HPP
#define XML_PARSER_HPP

#include "core/document.hpp"
#include "core/element.hpp"
#include "parser/parser_options.hpp"

#include <filesystem>
#include <iosfwd>
#include <memory>
#include <string_view>

namespace beresta
{
    /*
    ** XmlParser — DOM-парсер, публичный фасад (паттерн Facade) над
    ** конвейером «кодировка -> сессия разбора -> DomBuilder ->
    ** XInclude -> проверка по XML Schema».
    **
    ** Парсер не имеет изменяемого состояния: каждый вызов parse() создаёт
    ** собственную сессию, поэтому один объект XmlParser можно безопасно
    ** использовать из нескольких потоков одновременно (при условии, что
    ** указанные в настройках resolver и errorHandler тоже потокобезопасны).
    **
    ** Вход — байты в любой поддерживаемой кодировке: она определяется по
    ** BOM и XML-декларации (или задаётся ParserOptions::forcedEncoding).
    */
    class XmlParser
    {
    public:
        explicit XmlParser(ParserOptions options = ParserOptions());

        // Разбирает документ из памяти; при ошибке — ParseException
        Document parse(std::string_view input) const;

        // Разбирает файл (базовый URI для относительных ссылок — путь к файлу)
        Document parseFile(const std::filesystem::path& path) const;

        // Разбирает поток до конца
        Document parse(std::istream& stream) const;

        // Разбирает фрагмент и возвращает автономный корневой элемент
        std::unique_ptr<Element> parseElement(std::string_view input) const;

        // Текущие настройки
        const ParserOptions& getOptions() const noexcept;

    private:
        // Общая часть всех вариантов parse: разбор, XInclude, проверка схемой
        Document parseWithOptions(std::string_view bytes, const ParserOptions& options) const;

        ParserOptions _options;
    };
}

#endif
