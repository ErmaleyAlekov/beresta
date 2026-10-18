#ifndef PARSE_PIPELINE_HPP
#define PARSE_PIPELINE_HPP

#include "parser/parser_options.hpp"
#include "sax/content_handler.hpp"

#include <filesystem>
#include <string_view>

namespace beresta::detail
{
    /*
    ** Общий конвейер для DOM и SAX: байты -> определение кодировки ->
    ** UTF-8 -> сессия разбора -> события обработчику. Ошибка кодировки
    ** тоже фатальна и сначала сообщается ErrorHandler.
    **
    ** The common pipeline for DOM and SAX: bytes -> encoding detection ->
    ** UTF-8 -> a parse session -> events to the handler. An encoding error
    ** is fatal too and is reported to the ErrorHandler first.
    */
    void runParse(std::string_view bytes, const ParserOptions& options, ContentHandler& handler);

    /*
    ** Разбор SAX с проверкой по схеме (если она задана): события проходят
    ** через валидатор к обработчику, дерево не строится. Ошибки схемы
    ** сообщаются после endDocument — в errorHandler или исключением.
    **
    ** SAX parsing with schema validation (if a schema is set): the events pass
    ** through the validator to the handler, no tree is built. Schema errors
    ** are reported after endDocument — into errorHandler or as an exception.
    */
    void runSaxParse(std::string_view bytes, const ParserOptions& options, ContentHandler& handler);

    // Настройки для разбора файла: базовый URI по умолчанию — путь к файлу
    // Settings for parsing a file: the default base URI is the file path
    ParserOptions optionsForFile(const ParserOptions& options, const std::filesystem::path& path);
}

#endif
