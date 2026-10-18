#include "parser/xml_parser.hpp"
#include "parser/dom_builder.hpp"
#include "parser/internal/parse_pipeline.hpp"
#include "parser/internal/post_processing.hpp"

namespace beresta
{
    // Текущие настройки
    // The current settings
    const ParserOptions& XmlParser::getOptions() const noexcept
    {
        return _options;
    }

    /*
    ** Разбор в DOM и последующая обработка дерева: подстановка XInclude
    ** и проверка по XML Schema (если они включены в настройках).
    ** При проверке схемой позиции узлов записываются всегда — иначе
    ** ошибки валидности нельзя было бы привязать к строкам файла.
    **
    ** Parsing into a DOM and subsequent tree processing: XInclude substitution
    ** and XML Schema validation (if they are enabled in the settings).
    ** With schema validation node positions are always recorded — otherwise
    ** validity errors could not be tied to the lines of the file.
    */
    Document XmlParser::parseWithOptions(std::string_view bytes, const ParserOptions& options) const
    {
        ParserOptions effective = options;

        if (effective.schema)
            effective.trackLocations = true;
        DomBuilder builder(effective);
        detail::runParse(bytes, effective, builder);
        Document document = builder.takeDocument();
        detail::postProcessDocument(document, effective);
        return document;
    }
}
