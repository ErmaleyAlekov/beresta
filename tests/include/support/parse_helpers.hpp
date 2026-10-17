#ifndef PARSE_HELPERS_HPP
#define PARSE_HELPERS_HPP

#include "beresta.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace support
{
    // Код ошибки разбора; NOT_FOUND — если разбор прошёл без ошибок
    beresta::DomErrorCode errorCodeOf(std::string_view input);

    // Число ошибок валидности при разборе с проверкой по DTD и сбором ошибок
    std::size_t countValidityErrors(const std::string& document);

    // Допустимо ли значение для встроенного типа XSD
    bool acceptsBuiltin(const char* typeName, const char* value);

    // Ошибки проверки документа xml схемой xsd
    std::vector<beresta::Diagnostic> schemaErrors(const std::string& xsd, const std::string& xml);
}

#endif
