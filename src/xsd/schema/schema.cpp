#include "xsd/schema.hpp"
#include "i18n/i18n.hpp"
#include "xsd/internal/schema_builder.hpp"
#include "exception/dom_exception.hpp"
#include "parser/xml_parser.hpp"

namespace beresta::xsd
{
    // Пустая схема заполняется SchemaBuilder
    // An empty schema is filled by SchemaBuilder
    Schema::Schema()
        : _simpleTypes(), _complexTypes(), _elements(), _globalElements(), _globalTypes()
    {
    }

    /*
    ** Документы схем регистрируются целиком (с include/import), затем
    ** строятся все глобальные компоненты. Указатели на XML нужны только
    ** во время построения: готовая схема хранит собственные копии данных.
    **
    ** The schema documents are registered whole (with include/import), then
    ** all global components are built. The pointers to the XML are needed only
    ** during the construction: the ready schema keeps its own copies of the data.
    */
    std::shared_ptr<const Schema> Schema::fromDocuments(const std::vector<const Document*>& documents,
        const SchemaLoadOptions& options)
    {
        auto schema = std::make_shared<Schema>();
        detail::SchemaBuilder builder(*schema, options);

        for (const Document* document : documents)
        {
            const Element* root = document->getRoot();
            if (root == nullptr || !detail::isSchemaElement(*root, "schema"))
                throw DomException(DomErrorCode::SCHEMA_ERROR,
                    i18n::format(i18n::Message::SCHEMA_ROOT_INVALID, {XSD_NAMESPACE_URI}));
            builder.addDocument(*root, options.baseUri, "");
        }
        builder.buildAll();
        return schema;
    }

    // Текст схемы разбирается с позициями — ошибки схемы укажут строку
    // The schema text is parsed with positions — schema errors will point to the line
    std::shared_ptr<const Schema> Schema::fromString(std::string_view text,
        const SchemaLoadOptions& options)
    {
        ParserOptions parserOptions;
        parserOptions.trackLocations = true;
        Document document = XmlParser(parserOptions).parse(text);

        return fromDocuments({&document}, options);
    }

    // Файл схемы; относительные schemaLocation разрешаются от его пути
    // A schema file; relative schemaLocations are resolved against its path
    std::shared_ptr<const Schema> Schema::fromFile(const std::filesystem::path& path,
        SchemaLoadOptions options)
    {
        ParserOptions parserOptions;
        parserOptions.trackLocations = true;
        Document document = XmlParser(parserOptions).parseFile(path);

        if (options.baseUri.empty())
            options.baseUri = path.u8string();
        return fromDocuments({&document}, options);
    }

    // Глобальное объявление элемента
    // A global element declaration
    const ElementDecl* Schema::findElement(std::string_view namespaceUri, std::string_view localName) const
    {
        auto found = _globalElements.find(Key(namespaceUri, localName));

        if (found == _globalElements.end())
            return nullptr;
        return found->second;
    }
}
