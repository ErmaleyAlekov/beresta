#include "parser/xml_parser.hpp"
#include "parser/dom_builder.hpp"
#include "parser/internal/parse_pipeline.hpp"
#include "parser/internal/post_processing.hpp"

namespace beresta
{
    // Текущие настройки
    const ParserOptions& XmlParser::getOptions() const noexcept
    {
        return _options;
    }

    /*
    ** Разбор в DOM и последующая обработка дерева: подстановка XInclude
    ** и проверка по XML Schema (если они включены в настройках).
    ** При проверке схемой позиции узлов записываются всегда — иначе
    ** ошибки валидности нельзя было бы привязать к строкам файла.
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
