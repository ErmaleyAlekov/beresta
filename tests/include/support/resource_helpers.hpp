#ifndef RESOURCE_HELPERS_HPP
#define RESOURCE_HELPERS_HPP

#include "beresta.hpp"

#include <cstddef>
#include <map>
#include <string>

namespace support
{
    // «Файлы» в памяти: systemId -> содержимое
    // "Files" in memory: systemId -> content
    using FileMap = std::map<std::string, std::string>;

    // Итог разбора: была ли фатальная ошибка, сколько ошибок валидности, текст корня
    // The parse outcome: whether there was a fatal error, how many validity errors, the root text
    struct ParseOutcome
    {
        bool fatal;
        std::size_t validityErrors;
        std::string rootText;
    };

    /*
    ** Разбор документа, чьи внешние DTD и сущности берутся из files
    ** (ключ — systemId как он записан в документе). Внешний DTD загружается,
    ** ошибки валидности собираются (при validate), фатальная ошибка не бросается,
    ** а отражается в ParseOutcome::fatal — так тест проверяет оба исхода одинаково.
    **
    ** Parses a document whose external DTDs and entities are taken from files
    ** (the key is the systemId as written in the document). The external DTD is loaded,
    ** validity errors are collected (with validate), a fatal error is not thrown
    ** but reflected in ParseOutcome::fatal — so a test checks both outcomes the same way.
    */
    ParseOutcome parseWithFiles(const std::string& document, const FileMap& files, bool validate);
}

#endif
