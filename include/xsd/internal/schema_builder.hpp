#ifndef XSD_SCHEMA_BUILDER_HPP
#define XSD_SCHEMA_BUILDER_HPP

#include "xsd/schema.hpp"

#include <cstddef>
#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace beresta::xsd::detail
{
    // Имя компонента схемы: (пространство имён, локальное имя)
    // The name of a schema component: (namespace, local name)
    using QualifiedKey = std::pair<std::string, std::string>;

    // Один документ схемы и его настройки по умолчанию
    // One schema document and its default settings
    struct SchemaSource
    {
        const Element* root;
        std::string targetNamespace;
        bool qualifiedElements;
        bool qualifiedAttributes;
        std::string baseUri;
    };

    // Ещё не построенное глобальное определение: XML-элемент и его документ
    // A global definition not built yet: the XML element and its document
    struct Definition
    {
        const Element* element;
        const SchemaSource* source;
    };

    /*
    ** SchemaBuilder — загрузчик схемы (паттерн Builder) в два прохода:
    ** 1) регистрация всех глобальных определений всех документов схемы
    **    (с xs:include / xs:import) по их полным именам;
    ** 2) ленивое построение компонентов по требованию с запоминанием.
    ** Ленивость решает ссылки «вперёд» и рекурсивные типы (элемент,
    ** содержащий элементы своего же типа): объект типа регистрируется
    ** ДО построения содержимого, поэтому ссылка на него уже существует.
    **
    ** SchemaBuilder — the schema loader (the Builder pattern) in two passes:
    ** 1) registering all global definitions of all schema documents
    **    (with xs:include / xs:import) by their full names;
    ** 2) lazy on-demand construction of components with memoization.
    ** Laziness resolves forward references and recursive types (an element
    ** containing elements of its own type): the type object is registered
    ** BEFORE its content is built, so a reference to it already exists.
    */
    class SchemaBuilder
    {
    public:
        SchemaBuilder(Schema& schema, const SchemaLoadOptions& options);

        // Добавляет документ схемы (chameleon — пространство для схемы без targetNamespace)
        // Adds a schema document (chameleon — the namespace for a schema without targetNamespace)
        void addDocument(const Element& root, const std::string& baseUri, const std::string& chameleon);

        // Строит все глобальные элементы и типы
        // Builds all global elements and types
        void buildAll();

    private:
        // ---- Регистрация (schema_builder_register.cpp) ----
        // ---- Registration (schema_builder_register.cpp) ----
        void registerDefinition(const Element& child, const SchemaSource& source);
        void loadReferenced(const Element& directive, const SchemaSource& source);
        std::map<QualifiedKey, Definition>* registryFor(std::string_view kind);
        const Definition* findDefinition(std::string_view kind, const QualifiedKey& key);

        // ---- Имена (schema_builder_names.cpp) ----
        // ---- Names (schema_builder_names.cpp) ----
        QualifiedKey resolveQName(const Element& context, std::string_view value) const;
        TypeRef resolveType(const QualifiedKey& key);
        std::string localNamespace(const Element& definition, const SchemaSource& source,
            bool attribute) const;
        [[noreturn]] void fail(const Element& where, const std::string& message) const;

        // ---- Элементы (schema_builder_elements.cpp) ----
        // ---- Elements (schema_builder_elements.cpp) ----
        const ElementDecl* globalElement(const QualifiedKey& key);
        ElementDecl* buildElement(const Element& definition, const SchemaSource& source, bool global);
        TypeRef elementType(const Element& definition, const SchemaSource& source);
        void readValueConstraints(const Element& definition, std::optional<std::string>& defaultValue,
            std::optional<std::string>& fixedValue) const;

        // ---- Ограничения идентичности (schema_builder_identity*.cpp) ----
        // ---- Identity constraints (schema_builder_identity*.cpp) ----
        void readIdentityConstraints(const Element& definition, const SchemaSource& source,
            ElementDecl& declaration);
        const IdentityConstraint* buildIdentityConstraint(const Element& definition,
            const SchemaSource& source);
        std::vector<xpath::XPathExpression> identityFields(const Element& definition) const;
        xpath::XPathExpression compileIdentityPath(const Element& part) const;
        void resolveKeyReferences();

        // ---- Простые типы (schema_builder_simple.cpp, schema_builder_facets.cpp) ----
        // ---- Simple types (schema_builder_simple.cpp, schema_builder_facets.cpp) ----
        const SimpleType* buildSimpleType(const Element& definition, const SchemaSource& source,
            const QualifiedKey& name);
        const SimpleType* simpleTypeReference(const Element& owner, std::string_view attribute,
            const SchemaSource& source);
        void buildRestriction(SimpleType& type, const Element& restriction, const SchemaSource& source);
        void buildMembers(SimpleType& type, const Element& definition, const SchemaSource& source);
        void applyFacets(SimpleType& type, const Element& restriction);

        // ---- Составные типы (schema_builder_complex.cpp) ----
        // ---- Complex types (schema_builder_complex.cpp) ----
        const ComplexType* buildComplexType(const Element& definition, const SchemaSource& source,
            const QualifiedKey& name);
        void buildTypeBody(ComplexType& type, const Element& body, const SchemaSource& source,
            bool mixed);
        const Element& derivationOf(const Element& content, bool& extension) const;
        const ComplexType* completeBase(const Element& derivation, TypeRef& base);

        // ---- Наследование (schema_builder_derivation.cpp) ----
        // ---- Derivation (schema_builder_derivation.cpp) ----
        void buildSimpleContent(ComplexType& type, const Element& simpleContent, const SchemaSource& source);
        void buildComplexContent(ComplexType& type, const Element& complexContent,
            const SchemaSource& source, bool mixed);
        void prependBaseParticle(ComplexType& type, const ComplexType& base);

        // ---- Частицы (schema_builder_particles.cpp, schema_builder_occurs.cpp) ----
        // ---- Particles (schema_builder_particles.cpp, schema_builder_occurs.cpp) ----
        content::Particle buildParticle(const Element& definition, const SchemaSource& source,
            ComplexType& owner, std::size_t depth);
        content::Particle buildGroupReference(const Element& reference, ComplexType& owner,
            std::size_t depth);
        content::Particle buildElementParticle(const Element& definition, const SchemaSource& source,
            ComplexType& owner);
        content::Particle buildWildcard(const Element& definition, const SchemaSource& source,
            ComplexType& owner);
        void readOccurs(const Element& definition, content::Particle& particle) const;

        // ---- Атрибуты (schema_builder_attributes.cpp) ----
        // ---- Attributes (schema_builder_attributes.cpp) ----
        void buildAttributes(const Element& parent, const SchemaSource& source, ComplexType& owner,
            std::size_t depth);
        AttributeUse buildAttributeUse(const Element& definition, const SchemaSource& source,
            bool global);
        const Element& resolveAttributeReference(const Element& definition, AttributeUse& use,
            const SchemaSource*& typeSource);
        void addAttribute(ComplexType& owner, AttributeUse use) const;

        Schema& _schema;
        SchemaLoadOptions _options;
        std::deque<SchemaSource> _sources;
        std::vector<std::unique_ptr<Document>> _loaded;
        std::set<std::string> _loadedUris;
        std::map<std::string, std::map<QualifiedKey, Definition>> _definitions;
        std::map<QualifiedKey, ElementDecl*> _builtElements;
        std::map<QualifiedKey, TypeRef> _builtTypes;
        std::set<const ComplexType*> _incomplete;
        std::map<const Element*, IdentityConstraint*> _builtIdentities;
    };

    // ---- Обход XML схемы (schema_xml_helpers.cpp) ----
    // ---- Traversing the schema XML (schema_xml_helpers.cpp) ----

    // Дочерние элементы в пространстве XSD, кроме xs:annotation
    // Child elements in the XSD namespace, except xs:annotation
    std::vector<const Element*> schemaChildren(const Element& parent);

    // Первый дочерний элемент XSD с указанным локальным именем или nullptr
    // The first XSD child element with the given local name or nullptr
    const Element* firstSchemaChild(const Element& parent, std::string_view localName);

    // Является ли элемент компонентом XSD с указанным локальным именем
    // Whether the element is an XSD component with the given local name
    bool isSchemaElement(const Element& element, std::string_view localName);

    // Значение атрибута или запасное значение
    // The attribute value or a fallback value
    std::string attributeOr(const Element& element, std::string_view name, std::string_view fallback);

    // Режим processContents у xs:any / xs:anyAttribute (по умолчанию strict)
    // The processContents mode of xs:any / xs:anyAttribute (strict by default)
    ProcessContents parseProcessContents(const Element& wildcard);

    // Префиксы, объявленные на элементе схемы и его предках (для XPath селекторов)
    // The prefixes declared on the schema element and its ancestors (for XPath selectors)
    xpath::XPathContext namespacesInScope(const Element& element);
}

#endif
