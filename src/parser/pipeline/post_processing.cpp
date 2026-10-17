#include "parser/internal/post_processing.hpp"
#include "parser/error_handler.hpp"
#include "xinclude/xinclude.hpp"
#include "xsd/schema_validator.hpp"

namespace beresta::detail
{
    /*
    ** Ошибки схемы — ошибки валидности: с ErrorHandler они собираются,
    ** без него первая же ошибка прерывает разбор исключением.
    */
    static void validateWithSchema(const Document& document, const ParserOptions& options)
    {
        xsd::SchemaValidator validator(options.schema);

        for (const Diagnostic& diagnostic : validator.validate(document))
        {
            ParseException problem(diagnostic.message, diagnostic.position, DomErrorCode::VALIDATION_ERROR);
            if (options.errorHandler == nullptr)
                throw problem;
            options.errorHandler->error(problem);
        }
    }

    // Порядок важен: схема проверяет документ уже после подстановки включений
    void postProcessDocument(Document& document, const ParserOptions& options)
    {
        if (options.processXInclude)
            xinclude::processXIncludes(document, options);
        if (options.schema)
            validateWithSchema(document, options);
    }
}
