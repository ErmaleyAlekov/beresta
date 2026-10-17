#include "xsd/internal/schema_builder.hpp"

namespace beresta::xsd::detail
{
    // Строитель заполняет переданную схему
    SchemaBuilder::SchemaBuilder(Schema& schema, const SchemaLoadOptions& options)
        : _schema(schema), _options(options), _sources(), _loaded(), _loadedUris(),
          _definitions(), _builtElements(), _builtTypes(), _incomplete()
    {
    }

    /*
    ** Регистрация документа: значения по умолчанию из xs:schema, затем
    ** директивы include/import (загружают другие документы) и глобальные
    ** определения. «Хамелеонная» схема без targetNamespace, подключённая
    ** через include, получает пространство имён включающей схемы.
    */
    void SchemaBuilder::addDocument(const Element& root, const std::string& baseUri,
        const std::string& chameleon)
    {
        _sources.push_back(SchemaSource{&root, attributeOr(root, "targetNamespace", chameleon),
            attributeOr(root, "elementFormDefault", "unqualified") == "qualified",
            attributeOr(root, "attributeFormDefault", "unqualified") == "qualified", baseUri});
        const SchemaSource& source = _sources.back();
        for (const Element* child : schemaChildren(root))
        {
            if (isSchemaElement(*child, "include") || isSchemaElement(*child, "import"))
                loadReferenced(*child, source);
            else if (isSchemaElement(*child, "redefine") || isSchemaElement(*child, "override"))
                fail(*child, "xs:redefine и xs:override не поддерживаются");
            else
                registerDefinition(*child, source);
        }
    }

    // Все глобальные элементы и типы строятся сразу, чтобы ошибки схемы проявились при загрузке
    void SchemaBuilder::buildAll()
    {
        std::vector<QualifiedKey> elements;
        std::vector<QualifiedKey> types;

        for (const auto& entry : _definitions["element"])
            elements.push_back(entry.first);
        for (const auto& entry : _definitions["complexType"])
            types.push_back(entry.first);
        for (const auto& entry : _definitions["simpleType"])
            types.push_back(entry.first);
        for (const QualifiedKey& key : elements)
            globalElement(key);
        for (const QualifiedKey& key : types)
            resolveType(key);
    }
}
