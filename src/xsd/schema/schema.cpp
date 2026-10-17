#include "xsd/schema.hpp"
#include "xsd/internal/schema_builder.hpp"
#include "exception/dom_exception.hpp"
#include "parser/xml_parser.hpp"

namespace beresta::xsd
{
    // Пустая схема заполняется SchemaBuilder
    Schema::Schema()
        : _simpleTypes(), _complexTypes(), _elements(), _globalElements(), _globalTypes()
    {
    }

    /*
    ** Документы схем регистрируются целиком (с include/import), затем
    ** строятся все глобальные компоненты. Указатели на XML нужны только
    ** во время построения: готовая схема хранит собственные копии данных.
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
                    "корневой элемент схемы должен быть xs:schema из " + std::string(XSD_NAMESPACE_URI));
            builder.addDocument(*root, options.baseUri, "");
        }
        builder.buildAll();
        return schema;
    }

    // Текст схемы разбирается с позициями — ошибки схемы укажут строку
    std::shared_ptr<const Schema> Schema::fromString(std::string_view text,
        const SchemaLoadOptions& options)
    {
        ParserOptions parserOptions;
        parserOptions.trackLocations = true;
        Document document = XmlParser(parserOptions).parse(text);

        return fromDocuments({&document}, options);
    }

    // Файл схемы; относительные schemaLocation разрешаются от его пути
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
    const ElementDecl* Schema::findElement(std::string_view namespaceUri, std::string_view localName) const
    {
        auto found = _globalElements.find(Key(namespaceUri, localName));

        if (found == _globalElements.end())
            return nullptr;
        return found->second;
    }
}
