#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    // Строитель заполняет переданную схему
    // The builder fills the given schema
    SchemaBuilder::SchemaBuilder(Schema& schema, const SchemaLoadOptions& options)
        : _schema(schema), _options(options), _sources(), _loaded(), _loadedUris(), _loadedBy(),
          _definitions(), _builtElements(), _builtLocals(), _builtTypes(), _incomplete(), _builtIdentities(),
          _redefinitions(), _builtOriginals(), _overridden(), _headTyped(), _valueChecks()
    {
    }

    /*
    ** Регистрация документа: значения по умолчанию из xs:schema, затем
    ** директивы include/import (загружают другие документы),
    ** redefine/override (загружают и подменяют компоненты) и глобальные
    ** определения. «Хамелеонная» схема без targetNamespace, подключённая
    ** через include / redefine / override, получает пространство имён
    ** включающей схемы. Документ, чей xs:schema исключён vc:minVersion /
    ** vc:maxVersion, ничего не добавляет. URI документа запоминается сразу:
    ** include / redefine, ведущий обратно к корневому документу (цикл
    ** через другой файл), не загрузит его второй раз.
    **
    ** Registering a document: the defaults from xs:schema, then the
    ** include/import directives (they load other documents),
    ** redefine/override (they load and replace components) and the global
    ** definitions. A "chameleon" schema without targetNamespace included
    ** through include / redefine / override gets the namespace of the
    ** including schema. A document whose xs:schema is excluded by
    ** vc:minVersion / vc:maxVersion contributes nothing. The document URI is
    ** remembered at once: an include / redefine leading back to the root document
    ** (a cycle through another file) does not load it a second time.
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
        if (!baseUri.empty())
            _loadedUris.insert(baseUri);
        if (!versionAccepted(root))
            return;
        checkDocumentStructure(root);
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
    ** Все глобальные элементы, типы и группы строятся сразу, чтобы ошибки
    ** схемы проявились при загрузке. Группы замены вычисляются, когда построены
    ** все элементы (член группы может быть объявлен где угодно). Ссылки
    ** keyref связываются последними: ключ, на который ссылается keyref,
    ** может быть объявлен в любом месте схемы. Значения default / fixed
    ** элементов проверяются по типу в самом конце, когда все типы достроены.
    **
    ** All global elements, types and groups are built at once so that schema
    ** errors show up at loading. Substitution groups are computed when all the
    ** elements are built (a group member may be declared anywhere). keyref
    ** references are bound last: the key a keyref refers to may be declared
    ** anywhere in the schema. The default / fixed values of elements are checked
    ** by the type at the very end, when all types are complete.
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
        buildGlobalAttributes();
        buildNamedGroups();
        buildSubstitutionGroups();
        resolveKeyReferences();
        checkElementValues();
    }

    /*
    ** Глобальные объявления атрибутов (тип, default, fixed) копируются в
    ** схему: по ним xs:anyAttribute с lax / strict проверяет атрибуты,
    ** не объявленные в типе (cvc-wildcard, cvc-assess-attr).
    **
    ** Global attribute declarations (the type, default, fixed) are copied into
    ** the schema: by them xs:anyAttribute with lax / strict validates attributes
    ** not declared in the type (cvc-wildcard, cvc-assess-attr).
    */
    void SchemaBuilder::buildGlobalAttributes()
    {
        for (const auto& entry : _definitions["attribute"])
        {
            AttributeUse use = buildAttributeUse(*entry.second.element, *entry.second.source, true);
            _schema._globalAttributes[entry.first] = std::move(use);
        }
    }
}
