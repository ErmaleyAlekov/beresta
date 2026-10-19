#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    // Строитель заполняет переданную схему
    // The builder fills the given schema
    SchemaBuilder::SchemaBuilder(Schema& schema, const SchemaLoadOptions& options)
        : _schema(schema), _options(options), _sources(), _loaded(), _loadedUris(),
          _definitions(), _builtElements(), _builtTypes(), _incomplete(), _builtIdentities(),
          _redefinitions(), _builtOriginals(), _overridden(), _headTyped()
    {
    }

    /*
    ** Регистрация документа: значения по умолчанию из xs:schema, затем
    ** директивы include/import (загружают другие документы),
    ** redefine/override (загружают и подменяют компоненты) и глобальные
    ** определения. «Хамелеонная» схема без targetNamespace, подключённая
    ** через include / redefine / override, получает пространство имён
    ** включающей схемы.
    **
    ** Registering a document: the defaults from xs:schema, then the
    ** include/import directives (they load other documents),
    ** redefine/override (they load and replace components) and the global
    ** definitions. A "chameleon" schema without targetNamespace included
    ** through include / redefine / override gets the namespace of the
    ** including schema.
    */
    void SchemaBuilder::addDocument(const Element& root, const std::string& baseUri,
        const std::string& chameleon)
    {
        _sources.push_back(SchemaSource{&root, attributeOr(root, "targetNamespace", chameleon),
            attributeOr(root, "elementFormDefault", "unqualified") == "qualified",
            attributeOr(root, "attributeFormDefault", "unqualified") == "qualified", baseUri,
            derivationAttribute(root, "blockDefault", 0, ELEMENT_BLOCK_ALL),
            derivationAttribute(root, "finalDefault", 0, SCHEMA_FINAL_ALL)});
        const SchemaSource& source = _sources.back();
        for (const Element* child : schemaChildren(root))
        {
            if (isSchemaElement(*child, "include") || isSchemaElement(*child, "import"))
                loadReferenced(*child, source);
            else if (isSchemaElement(*child, "redefine") || isSchemaElement(*child, "override"))
                loadRedefinition(*child, source);
            else
                registerDefinition(*child, source);
        }
    }

    /*
    ** Все глобальные элементы и типы строятся сразу, чтобы ошибки схемы
    ** проявились при загрузке. Группы замены вычисляются, когда построены
    ** все элементы (член группы может быть объявлен где угодно). Ссылки
    ** keyref связываются последними: ключ, на который ссылается keyref,
    ** может быть объявлен в любом месте схемы.
    **
    ** All global elements and types are built at once so that schema errors
    ** show up at loading. Substitution groups are computed when all the
    ** elements are built (a group member may be declared anywhere). keyref
    ** references are bound last: the key a keyref refers to may be declared
    ** anywhere in the schema.
    */
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
        buildSubstitutionGroups();
        resolveKeyReferences();
    }
}
