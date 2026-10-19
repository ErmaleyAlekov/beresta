#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "parser/xml_parser.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Глобальное определение регистрируется под полным именем; дубликат —
    ** ошибка схемы. Имя, уже занятое через xs:override, пропускается:
    ** это и есть подменённое определение подключаемого документа.
    **
    ** A global definition is registered under its full name; a duplicate is
    ** a schema error. A name already taken through xs:override is skipped:
    ** this is exactly the replaced definition of the included document.
    */
    void SchemaBuilder::registerDefinition(const Element& child, const SchemaSource& source)
    {
        std::string kind(child.getLocalName());
        std::map<QualifiedKey, Definition>* registry = registryFor(kind);

        if (registry == nullptr)
            return;
        std::string name = attributeOr(child, "name", "");
        if (name.empty())
            fail(child, i18n::format(i18n::Message::SCHEMA_GLOBAL_NO_NAME, {kind}));
        QualifiedKey key(source.targetNamespace, name);
        if (_overridden.count(std::make_pair(kind, key)) != 0)
            return;
        if (!registry->emplace(key, Definition{&child, &source}).second)
            fail(child, i18n::format(i18n::Message::SCHEMA_REDEFINITION, {kind, name}));
    }

    /*
    ** xs:include / xs:import / xs:redefine / xs:override: документ
    ** загружается через резолвер один раз (защита от циклических
    ** включений) и регистрируется как обычный. Все, кроме import, ведут
    ** себя как include (в том числе «хамелеонные» схемы).
    ** xs:import без schemaLocation допустим: схема того пространства имён
    ** должна быть передана в Schema::fromDocuments отдельным документом.
    ** Результат — URI документа (пусто у import без адреса): по нему
    ** xs:redefine проверяет, чьи компоненты он переопределяет.
    **
    ** xs:include / xs:import / xs:redefine / xs:override: a document is
    ** loaded through the resolver once (protection against circular
    ** inclusions) and registered as an ordinary one. All except import behave
    ** like include (including "chameleon" schemas).
    ** xs:import without schemaLocation is allowed: the schema of that namespace
    ** must be passed to Schema::fromDocuments as a separate document.
    ** The result is the URI of the document (empty for an import without a location):
    ** xs:redefine checks by it whom its components belong to.
    */
    std::string SchemaBuilder::loadReferenced(const Element& directive, const SchemaSource& source)
    {
        std::string location = attributeOr(directive, "schemaLocation", "");
        bool include = !isSchemaElement(directive, "import");

        if (location.empty() && !include)
            return std::string();
        if (location.empty() || _options.resolver == nullptr)
            fail(directive, i18n::format(i18n::Message::SCHEMA_LOAD_NEEDS_RESOLVER, {location}));
        std::optional<Resource> resource = _options.resolver->resolve(
            ResourceRequest{ResourceKind::SCHEMA, "", location, source.baseUri});
        if (!resource.has_value())
            fail(directive, i18n::format(i18n::Message::SCHEMA_LOAD_FAILED, {location}));
        if (!_loadedUris.insert(resource->uri).second)
            return resource->uri;
        _loadedBy[resource->uri] = source.baseUri;
        ParserOptions parserOptions;
        parserOptions.trackLocations = true;
        _loaded.push_back(std::make_unique<Document>(XmlParser(parserOptions).parse(resource->content)));
        const Element* root = _loaded.back()->getRoot();
        if (root == nullptr || !isSchemaElement(*root, "schema"))
            fail(directive, i18n::format(i18n::Message::SCHEMA_NOT_A_SCHEMA, {location}));
        std::string chameleon;
        if (include)
            chameleon = source.targetNamespace;
        addDocument(*root, resource->uri, chameleon);
        return resource->uri;
    }

    // Реестры глобальных определений по видам компонентов
    // Registries of global definitions by component kind
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
    // A definition by kind and name or nullptr
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
