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

    // Один документ схемы и его настройки по умолчанию (включая blockDefault / finalDefault)
    // One schema document and its default settings (including blockDefault / finalDefault)
    struct SchemaSource
    {
        const Element* root;
        std::string targetNamespace;
        bool qualifiedElements;
        bool qualifiedAttributes;
        std::string baseUri;
        DerivationSet blockDefault;
        DerivationSet finalDefault;
    };

    // Ещё не построенное глобальное определение: XML-элемент и его документ
    // A global definition not built yet: the XML element and its document
    struct Definition
    {
        const Element* element;
        const SchemaSource* source;
    };

    /*
    ** Компонент, переопределённый через xs:redefine: вид, полное имя и
    ** прежнее определение. Ссылка на собственное имя внутри нового
    ** определения (base у типа, ref у группы) ведёт к прежнему, все
    ** остальные ссылки во всей схеме — к новому.
    **
    ** A component redefined through xs:redefine: the kind, the full name and
    ** the former definition. A reference to its own name inside the new
    ** definition (base of a type, ref of a group) leads to the former one, all
    ** the other references in the whole schema lead to the new one.
    */
    struct Redefinition
    {
        std::string kind;
        QualifiedKey key;
        Definition original;
    };

    // Все способы наследования для #all: у элемента в block, у элемента и составного типа в final, у простого типа
    // All derivation methods for #all: of an element in block, of an element and a complex type in final, of a simple type
    inline constexpr DerivationSet ELEMENT_BLOCK_ALL = DERIVATION_EXTENSION | DERIVATION_RESTRICTION
        | DERIVATION_SUBSTITUTION;
    inline constexpr DerivationSet COMPLEX_DERIVATIONS_ALL = DERIVATION_EXTENSION | DERIVATION_RESTRICTION;
    inline constexpr DerivationSet SIMPLE_FINAL_ALL = DERIVATION_RESTRICTION | DERIVATION_LIST | DERIVATION_UNION;
    inline constexpr DerivationSet SCHEMA_FINAL_ALL = COMPLEX_DERIVATIONS_ALL | SIMPLE_FINAL_ALL;

    /*
    ** Место объявления xs:element — от него зависит, какие атрибуты
    ** допустимы (схема для схем: topLevelElement, localElement и ссылка ref).
    **
    ** The place of an xs:element declaration — it decides which attributes
    ** are allowed (the schema for schemas: topLevelElement, localElement and a ref reference).
    */
    enum class ElementPlace
    {
        GLOBAL_ELEMENT,     // прямой ребёнок xs:schema / a direct child of xs:schema
        LOCAL_ELEMENT,      // объявление внутри модели содержимого / a declaration inside a content model
        ELEMENT_REFERENCE   // <xs:element ref="..."/> внутри модели / inside a model
    };

    /*
    ** SchemaBuilder — загрузчик схемы (паттерн Builder) в два прохода:
    ** 1) регистрация всех глобальных определений всех документов схемы
    **    (с xs:include / xs:import) по их полным именам; xs:redefine и
    **    xs:override здесь же подменяют определения в реестре;
    ** 2) ленивое построение компонентов по требованию с запоминанием.
    ** Ленивость решает ссылки «вперёд» и рекурсивные типы (элемент,
    ** содержащий элементы своего же типа): объект типа регистрируется
    ** ДО построения содержимого, поэтому ссылка на него уже существует.
    ** Завершает загрузку вычисление групп замены (нужны все элементы).
    **
    ** SchemaBuilder — the schema loader (the Builder pattern) in two passes:
    ** 1) registering all global definitions of all schema documents
    **    (with xs:include / xs:import) by their full names; xs:redefine and
    **    xs:override replace definitions in the registry right here;
    ** 2) lazy on-demand construction of components with memoization.
    ** Laziness resolves forward references and recursive types (an element
    ** containing elements of its own type): the type object is registered
    ** BEFORE its content is built, so a reference to it already exists.
    ** Loading ends with computing the substitution groups (all elements are needed).
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
        /*
        ** Сборка схемы рекурсивна: частица -> локальный элемент -> анонимный
        ** тип -> частица..., тип -> базовый тип, простой тип -> вложенный
        ** простой тип. Предел depth у частиц считает только одну модель, а на
        ** анонимном типе начинается заново, поэтому глубину всей цепочки
        ** считает общий счётчик _nesting. NestingGuard (RAII) увеличивает его
        ** при входе в buildParticle / buildElement / buildComplexType /
        ** buildSimpleType / buildAttributes и уменьшает при выходе, в том числе
        ** при исключении. Сверх MAX_NESTING — ошибка схемы SCHEMA_TOO_DEEP, а не
        ** переполнение стека. 256 уровней — около 64 вложенных локальных
        ** элементов с анонимными типами: с запасом для настоящих схем и
        ** безопасно даже для потоков с маленьким стеком.
        **
        ** Building a schema is recursive: a particle -> a local element -> an anonymous
        ** type -> a particle..., a type -> its base type, a simple type -> a nested
        ** simple type. The depth limit of particles counts only one model and starts
        ** anew at an anonymous type, so the depth of the whole chain is counted by the
        ** common _nesting counter. NestingGuard (RAII) increases it on entering
        ** buildParticle / buildElement / buildComplexType / buildSimpleType /
        ** buildAttributes and decreases it on leaving, including on an exception.
        ** Beyond MAX_NESTING it is the SCHEMA_TOO_DEEP schema error, not a stack
        ** overflow. 256 levels are about 64 nested local elements with anonymous types:
        ** plenty for real schemas and safe even for threads with a small stack.
        */
        static constexpr std::size_t MAX_NESTING = 256;

        /*
        ** Предел вложенности элементов самого документа схемы — как у парсера
        ** по умолчанию (maxDepth = 1024). Документ глубже не собрать при
        ** MAX_NESTING (уровень сборки — не больше четырёх уровней XML), а обход
        ** такого документа стоил бы O(n · глубина): пространство имён элемента
        ** ищется подъёмом к корню. Проверяется первым, в checkDocumentIds.
        **
        ** The element nesting limit of the schema document itself — as the parser's
        ** default (maxDepth = 1024). A deeper document cannot be built under
        ** MAX_NESTING (a building level is at most four XML levels), and a traversal
        ** of such a document would cost O(n · depth): the namespace of an element
        ** is looked up by climbing to the root. It is checked first, in checkDocumentIds.
        */
        static constexpr std::size_t MAX_DOCUMENT_DEPTH = 4 * MAX_NESTING;

        class NestingGuard
        {
        public:
            NestingGuard(SchemaBuilder& builder, const Element& where);
            ~NestingGuard();
            NestingGuard(const NestingGuard&) = delete;
            NestingGuard& operator=(const NestingGuard&) = delete;

        private:
            SchemaBuilder& _builder;
        };

        // Глобальные объявления атрибутов — в схему, для xs:anyAttribute (schema_builder.cpp)
        // Global attribute declarations — into the schema, for xs:anyAttribute (schema_builder.cpp)
        void buildGlobalAttributes();

        // ---- Регистрация (schema_builder_register.cpp) ----
        // ---- Registration (schema_builder_register.cpp) ----
        void registerDefinition(const Element& child, const SchemaSource& source);
        std::string loadReferenced(const Element& directive, const SchemaSource& source);
        std::map<QualifiedKey, Definition>* registryFor(std::string_view kind);
        const Definition* findDefinition(std::string_view kind, const QualifiedKey& key);

        // ---- xs:redefine и xs:override (schema_builder_redefine.cpp, schema_builder_original.cpp) ----
        // ---- xs:redefine and xs:override (schema_builder_redefine.cpp, schema_builder_original.cpp) ----
        void loadRedefinition(const Element& directive, const SchemaSource& source);
        void redefineDefinition(const Element& child, const SchemaSource& source, const std::string& targetUri);
        void checkSelfReferences(const Element& child, const QualifiedKey& key) const;
        std::vector<const Element*> selfReferences(const Element& component, const QualifiedKey& key) const;
        bool loadedWithin(const std::string& uri, const std::string& ancestor) const;
        void buildNamedGroups();
        void checkNamedGroup(const Element& group) const;
        void overrideDefinition(const Element& child, const SchemaSource& source);
        void checkSelfDerivation(const Element& child, const QualifiedKey& key) const;
        const Definition* originalFor(const Element& reference, std::string_view kind,
            const QualifiedKey& key) const;
        const Definition* referencedDefinition(const Element& reference, std::string_view kind,
            const QualifiedKey& key);
        TypeRef referencedType(const Element& context, std::string_view value);

        // ---- final, block и группы замены (schema_builder_final.cpp, schema_builder_substitution.cpp) ----
        // ---- final, block and substitution groups (schema_builder_final.cpp, schema_builder_substitution.cpp) ----
        DerivationSet derivationAttribute(const Element& definition, std::string_view attribute,
            DerivationSet fallback, DerivationSet all) const;
        void checkFinal(const Element& where, DerivationSet finalSet, const std::string& baseName,
            DerivationSet method) const;
        void buildSubstitutionGroups();
        void inheritHeadTypes();
        void checkSubstitutionFinal(const ElementDecl& member);
        void addSubstitute(const ElementDecl& member);

        // ---- Лексика атрибутов схемы (schema_builder_lexical.cpp) ----
        // ---- The lexical form of schema attributes (schema_builder_lexical.cpp) ----
        bool booleanAttribute(const Element& definition, std::string_view name, bool fallback) const;
        void checkElementAttributes(const Element& definition, ElementPlace place) const;

        // ---- Имена (schema_builder_names.cpp) ----
        // ---- Names (schema_builder_names.cpp) ----
        QualifiedKey resolveQName(const Element& context, std::string_view value) const;
        std::string chameleonNamespace(const Element& context) const;
        TypeRef resolveType(const QualifiedKey& key);
        std::string localNamespace(const Element& definition, const SchemaSource& source,
            bool attribute) const;
        [[noreturn]] void fail(const Element& where, const std::string& message) const;

        // ---- Строение документа схемы (schema_builder_structure.cpp) ----
        // ---- The structure of a schema document (schema_builder_structure.cpp) ----
        void checkDocumentIds(const Element& root) const;
        void checkIdAttribute(const Element& element, std::set<std::string>& seen) const;
        void checkChildOrder(const Element& element) const;
        void checkGroupChild(const Element& parent, const Element& child, std::size_t& models) const;
        void checkNCName(const Element& where, std::string_view attribute, const std::string& value) const;

        // ---- Элементы (schema_builder_elements.cpp) ----
        // ---- Elements (schema_builder_elements.cpp) ----
        const ElementDecl* globalElement(const QualifiedKey& key);
        ElementDecl* buildElement(const Element& definition, const SchemaSource& source, bool global);
        void readElementFlags(const Element& definition, const SchemaSource& source,
            ElementDecl& declaration) const;
        TypeRef elementType(const Element& definition, const SchemaSource& source, ElementDecl& declaration);
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
            const QualifiedKey& name, bool publish = true);
        void buildSimpleBody(SimpleType& type, const Element& definition, const SchemaSource& source);
        const SimpleType* simpleTypeReference(const Element& owner, std::string_view attribute,
            const SchemaSource& source);
        void buildRestriction(SimpleType& type, const Element& restriction, const SchemaSource& source);
        void buildMembers(SimpleType& type, const Element& definition, const SchemaSource& source);
        void applyFacets(SimpleType& type, const Element& restriction, const SchemaSource& source);

        // ---- Утверждения XSD 1.1 (schema_builder_assertions.cpp) ----
        // ---- XSD 1.1 assertions (schema_builder_assertions.cpp) ----
        std::shared_ptr<const Assertion> buildAssertion(const Element& definition,
            const SchemaSource& source) const;
        void readAssertions(const Element& body, const SchemaSource& source, ComplexType& type) const;
        void readAssertionFacets(SimpleType& type, const Element& restriction, const SchemaSource& source) const;

        // ---- Составные типы (schema_builder_complex.cpp) ----
        // ---- Complex types (schema_builder_complex.cpp) ----
        const ComplexType* buildComplexType(const Element& definition, const SchemaSource& source,
            const QualifiedKey& name, bool publish = true);
        void readTypeFlags(const Element& definition, const SchemaSource& source, ComplexType& type) const;
        void buildTypeBody(ComplexType& type, const Element& body, const SchemaSource& source,
            bool mixed);
        const Element& derivationOf(const Element& content, bool& extension) const;
        const ComplexType* completeBase(const Element& derivation, bool extension, ComplexType& type);

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
        std::optional<AttributeWildcard> buildAttributes(const Element& parent, const SchemaSource& source,
            ComplexType& owner, std::size_t depth);
        std::optional<AttributeWildcard> attributeGroupWildcard(const Element& reference, ComplexType& owner,
            std::size_t depth);
        AttributeUse buildAttributeUse(const Element& definition, const SchemaSource& source,
            bool global);
        const Element& resolveAttributeReference(const Element& definition, AttributeUse& use,
            const SchemaSource*& typeSource);
        void addAttribute(ComplexType& owner, AttributeUse use) const;

        // ---- Подстановочные атрибуты (schema_builder_wildcards.cpp) ----
        // ---- Attribute wildcards (schema_builder_wildcards.cpp) ----
        AttributeWildcard readAttributeWildcard(const Element& definition, const SchemaSource& source) const;
        void mergeGroupWildcard(std::optional<AttributeWildcard>& total,
            const std::optional<AttributeWildcard>& next, const Element& where) const;
        void applyAttributeWildcard(ComplexType& type, std::optional<AttributeWildcard> local) const;
        void inheritAttributeWildcard(ComplexType& type, const ComplexType& base, const Element& where) const;

        Schema& _schema;
        SchemaLoadOptions _options;
        std::deque<SchemaSource> _sources;
        std::vector<std::unique_ptr<Document>> _loaded;
        std::set<std::string> _loadedUris;
        std::map<std::string, std::string> _loadedBy;
        std::map<std::string, std::map<QualifiedKey, Definition>> _definitions;
        std::map<QualifiedKey, ElementDecl*> _builtElements;
        std::map<const Element*, ElementDecl*> _builtLocals;
        std::map<QualifiedKey, TypeRef> _builtTypes;
        std::set<const void*> _incomplete;
        std::map<const Element*, IdentityConstraint*> _builtIdentities;
        std::map<const Element*, Redefinition> _redefinitions;
        std::map<const Element*, TypeRef> _builtOriginals;
        std::set<std::pair<std::string, QualifiedKey>> _overridden;
        std::set<const ElementDecl*> _headTyped;
        std::size_t _nesting = 0;       // текущая глубина сборки / the current building depth
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

    /*
    ** Атрибут namespace у xs:any / xs:anyAttribute: ##any, ##other или
    ** перечень (##targetNamespace — целевое пространство, ##local — пустая
    ** строка). Реализация — schema_builder_wildcards.cpp.
    **
    ** The namespace attribute of xs:any / xs:anyAttribute: ##any, ##other or
    ** a list (##targetNamespace — the target namespace, ##local — an empty
    ** string). The implementation is in schema_builder_wildcards.cpp.
    */
    void readNamespaceConstraint(const Element& wildcard, const std::string& targetNamespace,
        content::WildcardMode& mode, std::vector<std::string>& namespaces);

    // Префиксы, объявленные на элементе схемы и его предках (для XPath селекторов)
    // The prefixes declared on the schema element and its ancestors (for XPath selectors)
    xpath::XPathContext namespacesInScope(const Element& element);

    /*
    ** Условное включение XSD 1.1 (schema_versioning.cpp): «Берёста» — процессор
    ** версии 1.1, поэтому компонент с vc:minVersion > 1.1 или vc:maxVersion <= 1.1
    ** пропускается при загрузке, как будто его нет в документе схемы.
    **
    ** XSD 1.1 conditional inclusion (schema_versioning.cpp): Beresta is a processor
    ** of version 1.1, so a component with vc:minVersion > 1.1 or vc:maxVersion <= 1.1
    ** is skipped at loading, as if it were not in the schema document.
    */
    bool versionAccepted(const Element& element);
}

#endif
