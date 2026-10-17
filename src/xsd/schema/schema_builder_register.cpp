#include "xsd/internal/schema_builder.hpp"
#include "exception/dom_exception.hpp"
#include "parser/xml_parser.hpp"

namespace beresta::xsd::detail
{
    // Глобальное определение регистрируется под полным именем; дубликат — ошибка схемы
    void SchemaBuilder::registerDefinition(const Element& child, const SchemaSource& source)
    {
        std::string kind(child.getLocalName());
        std::map<QualifiedKey, Definition>* registry = registryFor(kind);

        if (registry == nullptr)
            return;
        std::string name = attributeOr(child, "name", "");
        if (name.empty())
            fail(child, "у глобального xs:" + kind + " нет атрибута name");
        QualifiedKey key(source.targetNamespace, name);
        if (!registry->emplace(key, Definition{&child, &source}).second)
            fail(child, "повторное определение xs:" + kind + " \"" + name + "\"");
    }

    /*
    ** xs:include / xs:import: документ загружается через резолвер один раз
    ** (защита от циклических включений) и регистрируется как обычный.
    ** xs:import без schemaLocation допустим: схема того пространства имён
    ** должна быть передана в Schema::fromDocuments отдельным документом.
    */
    void SchemaBuilder::loadReferenced(const Element& directive, const SchemaSource& source)
    {
        std::string location = attributeOr(directive, "schemaLocation", "");
        bool include = isSchemaElement(directive, "include");

        if (location.empty() && !include)
            return;
        if (location.empty() || _options.resolver == nullptr)
            fail(directive, "для загрузки \"" + location + "\" нужен schemaLocation и ResourceResolver");
        std::optional<Resource> resource = _options.resolver->resolve(
            ResourceRequest{ResourceKind::SCHEMA, "", location, source.baseUri});
        if (!resource.has_value())
            fail(directive, "не удалось загрузить схему \"" + location + "\"");
        if (!_loadedUris.insert(resource->uri).second)
            return;
        ParserOptions parserOptions;
        parserOptions.trackLocations = true;
        _loaded.push_back(std::make_unique<Document>(XmlParser(parserOptions).parse(resource->content)));
        const Element* root = _loaded.back()->getRoot();
        if (root == nullptr || !isSchemaElement(*root, "schema"))
            fail(directive, "\"" + location + "\" не является XML Schema");
        std::string chameleon;
        if (include)
            chameleon = source.targetNamespace;
        addDocument(*root, resource->uri, chameleon);
    }

    // Реестры глобальных определений по видам компонентов
    std::map<QualifiedKey, Definition>* SchemaBuilder::registryFor(std::string_view kind)
    {
        static const char* const KINDS[] = {
            "element", "complexType", "simpleType", "attribute", "group", "attributeGroup"
        };

        for (const char* known : KINDS)
        {
            if (kind == known)
                return &_definitions[known];
        }
        return nullptr;
    }

    // Определение по виду и имени или nullptr
    const Definition* SchemaBuilder::findDefinition(std::string_view kind, const QualifiedKey& key)
    {
        std::map<QualifiedKey, Definition>* registry = registryFor(kind);

        if (registry == nullptr)
            return nullptr;
        auto found = registry->find(key);
        if (found == registry->end())
            return nullptr;
        return &found->second;
    }
}
