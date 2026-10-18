#include "parser/internal/post_processing.hpp"
#include "parser/error_handler.hpp"
#include "xinclude/xinclude.hpp"
#include "xsd/schema_validator.hpp"

namespace beresta::detail
{
    // Каждая ошибка — исключение ParseException с кодом VALIDATION_ERROR
    // Every error is a ParseException with the VALIDATION_ERROR code
    void reportSchemaDiagnostics(const std::vector<Diagnostic>& diagnostics, const ParserOptions& options)
    {
        for (const Diagnostic& diagnostic : diagnostics)
        {
            ParseException problem(diagnostic.message, diagnostic.position, DomErrorCode::VALIDATION_ERROR);
            if (options.errorHandler == nullptr)
                throw problem;
            options.errorHandler->error(problem);
        }
    }

    // XInclude меняет дерево после разбора, поэтому с ним схема проверяет готовое дерево
    // XInclude changes the tree after parsing, so with it the schema validates the ready tree
    bool validatesWhileParsing(const ParserOptions& options) noexcept
    {
        return options.schema != nullptr && !options.processXInclude;
    }

    // Порядок важен: схема проверяет документ уже после подстановки включений
    // The order matters: the schema validates the document after the inclusions are substituted
    void postProcessDocument(Document& document, const ParserOptions& options)
    {
        if (!options.processXInclude)
            return;
        xinclude::processXIncludes(document, options);
        if (options.schema)
            reportSchemaDiagnostics(xsd::SchemaValidator(options.schema).validate(document), options);
    }
}
