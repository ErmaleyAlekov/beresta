#ifndef POST_PROCESSING_HPP
#define POST_PROCESSING_HPP

#include "core/document.hpp"
#include "parser/error_handler.hpp"
#include "parser/parser_options.hpp"

#include <vector>

namespace beresta::detail
{
    /*
    ** Обработка готового дерева после разбора: XInclude (если
    ** processXInclude) и проверка по XML Schema — но только вместе с
    ** XInclude: схема обязана видеть дерево уже после подстановки
    ** включений. Без XInclude схема проверяется потоково, прямо при разборе.
    **
    ** Processing of the ready tree after parsing: XInclude (if
    ** processXInclude) and XML Schema validation — but only together with
    ** XInclude: the schema must see the tree already after the inclusions are
    ** substituted. Without XInclude the schema is validated in a stream, right during parsing.
    */
    void postProcessDocument(Document& document, const ParserOptions& options);

    // Проверяется ли схема прямо при разборе (есть схема и нет XInclude)
    // Whether the schema is validated right during parsing (there is a schema and no XInclude)
    bool validatesWhileParsing(const ParserOptions& options) noexcept;

    /*
    ** Ошибки схемы — ошибки валидности: с ErrorHandler они собираются,
    ** без него первая же ошибка прерывает разбор исключением.
    **
    ** Schema errors are validity errors: with an ErrorHandler they are collected,
    ** without it the very first error stops parsing with an exception.
    */
    void reportSchemaDiagnostics(const std::vector<Diagnostic>& diagnostics, const ParserOptions& options);
}

#endif
