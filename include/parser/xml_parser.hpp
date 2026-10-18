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
    **
    ** XmlParser — the DOM parser, a public facade (the Facade pattern) over the
    ** pipeline "encoding -> parse session -> DomBuilder ->
    ** XInclude -> XML Schema validation".
    ** The parser has no mutable state: every parse() call creates its own
    ** session, so one XmlParser object can be used safely from several
    ** threads at the same time (provided that the resolver and errorHandler
    ** given in the settings are thread-safe too).
    ** The input is bytes in any supported encoding: it is detected by the
    ** BOM and the XML declaration (or set by ParserOptions::forcedEncoding).
    */
    class XmlParser
    {
    public:
        explicit XmlParser(ParserOptions options = ParserOptions());

        // Разбирает документ из памяти; при ошибке — ParseException
        // Parses a document from memory; on error — ParseException
        Document parse(std::string_view input) const;

        // Разбирает файл (базовый URI для относительных ссылок — путь к файлу)
        // Parses a file (the base URI for relative references is the file path)
        Document parseFile(const std::filesystem::path& path) const;

        // Разбирает поток до конца
        // Parses a stream to the end
        Document parse(std::istream& stream) const;

        // Разбирает фрагмент и возвращает автономный корневой элемент
        // Parses a fragment and returns a standalone root element
        std::unique_ptr<Element> parseElement(std::string_view input) const;

        // Текущие настройки
        // The current settings
        const ParserOptions& getOptions() const noexcept;

    private:
        // Общая часть всех вариантов parse: разбор, XInclude, проверка схемой
        // The common part of all parse variants: parsing, XInclude, schema validation
        Document parseWithOptions(std::string_view bytes, const ParserOptions& options) const;

        ParserOptions _options;
    };
}

#endif
