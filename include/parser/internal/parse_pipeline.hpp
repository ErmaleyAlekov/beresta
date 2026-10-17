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
    */
    void runParse(std::string_view bytes, const ParserOptions& options, ContentHandler& handler);

    // Настройки для разбора файла: базовый URI по умолчанию — путь к файлу
    ParserOptions optionsForFile(const ParserOptions& options, const std::filesystem::path& path);
}

#endif
