#include "parser/xml_parser.hpp"
#include "parser/dom_builder.hpp"
#include "parser/internal/parse_pipeline.hpp"
#include "parser/internal/post_processing.hpp"
#include "xsd/validating_handler.hpp"

namespace beresta
{
    // Текущие настройки
    // The current settings
    const ParserOptions& XmlParser::getOptions() const noexcept
    {
        return _options;
    }

    /*
    ** Разбор в DOM. Со схемой (и без XInclude) события идут через
    ** валидатор-декоратор в DomBuilder: документ проверяется прямо во время
    ** разбора, одним проходом, а ключи — по готовому дереву. Позиции ошибок
    ** валидатор берёт у парсера сам, поэтому узлам они записываются, только
    ** если их просили (trackLocations). Проверке готового дерева после
    ** XInclude позиции узлов нужны — там они записываются всегда.
    **
    ** Parsing into a DOM. With a schema (and without XInclude) the events go through
    ** the decorator validator into DomBuilder: the document is validated right during
    ** parsing, in one pass, and the keys — over the ready tree. The validator takes the
    ** error positions from the parser itself, so they are recorded into the nodes only
    ** if requested (trackLocations). Validating the ready tree after
    ** XInclude needs node positions — there they are always recorded.
    */
    Document XmlParser::parseWithOptions(std::string_view bytes, const ParserOptions& options) const
    {
        ParserOptions effective = options;

        if (effective.schema && effective.processXInclude)
            effective.trackLocations = true;
        DomBuilder builder(effective);
        if (!detail::validatesWhileParsing(effective))
        {
            detail::runParse(bytes, effective, builder);
            Document document = builder.takeDocument();
            detail::postProcessDocument(document, effective);
            return document;
        }
        xsd::ValidatingHandler validator(effective.schema, &builder,
            xsd::validationSettingsFor(effective, xsd::IdentityEvaluation::ON_TREE));
        detail::runParse(bytes, effective, validator);
        Document document = builder.takeDocument();
        validator.checkIdentity(document);
        detail::reportSchemaDiagnostics(validator.takeDiagnostics(), effective);
        return document;
    }
}
