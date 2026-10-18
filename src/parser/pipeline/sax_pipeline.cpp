#include "parser/internal/parse_pipeline.hpp"
#include "parser/internal/post_processing.hpp"
#include "xsd/validating_handler.hpp"

namespace beresta::detail
{
    /*
    ** Без схемы — обычный разбор. Со схемой обработчик пользователя
    ** оборачивается валидатором-декоратором: память проверки — O(глубины),
    ** ключи — потоково. Пути в сообщениях получают номера одноимённых
    ** соседей лишь в конце документа, поэтому ошибки сообщаются после него.
    **
    ** Without a schema — ordinary parsing. With a schema the user's handler
    ** is wrapped with the decorator validator: the validation memory is O(depth),
    ** the keys — in a stream. The paths in messages get the numbers of same-named
    ** siblings only at the end of the document, so the errors are reported after it.
    */
    void runSaxParse(std::string_view bytes, const ParserOptions& options, ContentHandler& handler)
    {
        if (!options.schema)
        {
            runParse(bytes, options, handler);
            return;
        }
        xsd::ValidatingHandler validator(options.schema, &handler,
            xsd::validationSettingsFor(options, xsd::IdentityEvaluation::STREAMING));
        runParse(bytes, options, validator);
        reportSchemaDiagnostics(validator.takeDiagnostics(), options);
    }
}
