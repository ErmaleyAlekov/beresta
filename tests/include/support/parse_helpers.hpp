#ifndef PARSE_HELPERS_HPP
#define PARSE_HELPERS_HPP

#include "beresta.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace support
{
    // Код ошибки разбора; NOT_FOUND — если разбор прошёл без ошибок
    // The parse error code; NOT_FOUND if parsing succeeded without errors
    beresta::DomErrorCode errorCodeOf(std::string_view input);

    // Число ошибок валидности при разборе с проверкой по DTD и сбором ошибок
    // The number of validity errors when parsing with DTD validation and error collection
    std::size_t countValidityErrors(const std::string& document);

    // Допустимо ли значение для встроенного типа XSD
    // Whether a value is allowed for a built-in XSD type
    bool acceptsBuiltin(const char* typeName, const char* value);

    // Ошибки проверки документа xml схемой xsd
    // The errors of validating the document xml against the schema xsd
    std::vector<beresta::Diagnostic> schemaErrors(const std::string& xsd, const std::string& xml);

    // Схема из тела body внутри <xs:schema> (префикс xs — пространство XSD)
    // A schema from the body body inside <xs:schema> (the xs prefix is the XSD namespace)
    std::shared_ptr<const beresta::xsd::Schema> schemaFromBody(const std::string& body);
}

#endif
