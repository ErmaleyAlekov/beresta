#ifndef XSD_SCHEMA_HPP
#define XSD_SCHEMA_HPP

#include "content/particle.hpp"
#include "core/document.hpp"
#include "parser/resource_resolver.hpp"
#include "xsd/simple_type.hpp"
#include "xpath/xpath.hpp"

#include <cstdint>
#include <deque>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace beresta::xsd
{
    struct ComplexType;
    struct ElementDecl;
    struct IdentityConstraint;

    namespace detail
    {
        class SchemaBuilder;
    }

    // Ссылка на тип: простой, составной или (оба nullptr) xs:anyType — «что угодно»
    // A type reference: simple, complex or (both nullptr) xs:anyType — "anything"
    struct TypeRef
    {
        const SimpleType* simple = nullptr;
        const ComplexType* complex = nullptr;
    };

    /*
    ** Набор способов наследования — битовые флаги для атрибутов final и
    ** block (и их значений по умолчанию finalDefault / blockDefault).
    ** У типа derivationMethod — ровно один флаг: как он получен из базового.
    **
    ** A set of derivation methods — bit flags for the final and block
    ** attributes (and their defaults finalDefault / blockDefault).
    ** A type's derivationMethod is exactly one flag: how it was obtained from its base.
    */
    using DerivationSet = std::uint8_t;
    inline constexpr DerivationSet DERIVATION_EXTENSION = 1;
    inline constexpr DerivationSet DERIVATION_RESTRICTION = 2;
    inline constexpr DerivationSet DERIVATION_LIST = 4;
    inline constexpr DerivationSet DERIVATION_UNION = 8;
    inline constexpr DerivationSet DERIVATION_SUBSTITUTION = 16;

    // Режим проверки содержимого, допущенного шаблоном xs:any / xs:anyAttribute
    // The validation mode of content admitted by the xs:any / xs:anyAttribute wildcard
    enum class ProcessContents
    {
        STRICT_CHECK,   // объявление обязано найтись и проверяется / a declaration must be found and is validated
        LAX_CHECK,      // проверяется, если объявление найдено / validated if a declaration is found
        SKIP_CHECK      // не проверяется / not validated
    };

    // Вид содержимого составного типа
    // The content kind of a complex type
    enum class ContentType
    {
        EMPTY_CONTENT,  // ни детей, ни текста / neither children nor text
        SIMPLE_CONTENT, // только текст простого типа / only text of a simple type
        ELEMENT_ONLY,   // только элементы (пробелы между ними допустимы)
                        // only elements (spaces between them are allowed)
        MIXED_CONTENT   // элементы вперемешку с текстом / elements mixed with text
    };

    // Использование атрибута в составном типе
    // The use of an attribute in a complex type
    struct AttributeUse
    {
        std::string name;
        std::string namespaceUri;
        const SimpleType* type = nullptr;
        bool required = false;
        bool prohibited = false;
        std::optional<std::string> defaultValue;
        std::optional<std::string> fixedValue;
    };

    /*
    ** Составной тип: модель содержимого и атрибуты. baseType и
    ** derivationMethod описывают шаг наследования (у типа без
    ** complexContent / simpleContent — ограничение xs:anyType); по ним
    ** проверяется, выведен ли тип из xsi:type из объявленного.
    **
    ** A complex type: the content model and the attributes. baseType and
    ** derivationMethod describe the derivation step (a type without
    ** complexContent / simpleContent is a restriction of xs:anyType); they are used
    ** to check whether the xsi:type type is derived from the declared one.
    */
    struct ComplexType
    {
        std::string name;
        std::string namespaceUri;
        TypeRef baseType;
        DerivationSet derivationMethod = DERIVATION_RESTRICTION;
        DerivationSet finalSet = 0;
        DerivationSet blockSet = 0;
        bool isAbstract = false;
        ContentType content = ContentType::EMPTY_CONTENT;
        const SimpleType* simpleContent = nullptr;
        content::Particle particle;
        bool hasParticle = false;
        std::vector<const ElementDecl*> elements;
        std::vector<AttributeUse> attributes;
        bool anyAttribute = false;
        ProcessContents attributeWildcard = ProcessContents::STRICT_CHECK;
        bool hasElementWildcard = false;
        ProcessContents elementWildcard = ProcessContents::STRICT_CHECK;
    };

    // Вид ограничения идентичности
    // The kind of an identity constraint
    enum class IdentityKind
    {
        UNIQUE,     // xs:unique — значения уникальны там, где все поля есть
                    // xs:unique — the values are unique where all fields are present
        KEY,        // xs:key — поля обязательны, значения уникальны
                    // xs:key — the fields are mandatory, the values are unique
        KEYREF      // xs:keyref — значения обязаны совпадать с ключом refer
                    // xs:keyref — the values must match the refer key
    };

    /*
    ** Ограничение идентичности (xs:unique / xs:key / xs:keyref).
    ** selector выбирает целевые элементы относительно элемента-области,
    ** fields — значения, образующие ключ. Выражения компилируются движком
    ** XPath 1.0 один раз при загрузке схемы; префиксы в них разрешаются
    ** по объявлениям xmlns документа схемы (context.namespaces), а имя
    ** без префикса, как и положено в XSD 1.0, — вне пространств имён.
    **
    ** An identity constraint (xs:unique / xs:key / xs:keyref).
    ** selector selects the target elements relative to the scope element,
    ** fields are the values forming the key. The expressions are compiled by the
    ** XPath 1.0 engine once when the schema is loaded; the prefixes in them are resolved
    ** by the xmlns declarations of the schema document (context.namespaces), and a name
    ** without a prefix, as XSD 1.0 requires, is in no namespace.
    */
    struct IdentityConstraint
    {
        IdentityKind kind;
        std::string name;
        std::string namespaceUri;
        xpath::XPathExpression selector;
        std::vector<xpath::XPathExpression> fields;
        xpath::XPathContext context;
        std::pair<std::string, std::string> refer;
        const IdentityConstraint* referenced = nullptr;
    };

    /*
    ** Объявление элемента. substitutionHead — голова группы замены
    ** (атрибут substitutionGroup), substitutes — все глобальные элементы,
    ** которые допустимо поставить в документ вместо этого (транзитивно,
    ** без абстрактных и заблокированных block); вычисляется после
    ** загрузки всей схемы.
    **
    ** An element declaration. substitutionHead is the substitution group head
    ** (the substitutionGroup attribute), substitutes are all the global elements
    ** that may be put into a document instead of this one (transitively,
    ** without abstract ones and the ones blocked by block); it is computed after
    ** the whole schema is loaded.
    */
    struct ElementDecl
    {
        std::string name;
        std::string namespaceUri;
        TypeRef type;
        bool nillable = false;
        bool isAbstract = false;
        DerivationSet finalSet = 0;
        DerivationSet blockSet = 0;
        const ElementDecl* substitutionHead = nullptr;
        content::SubstitutionGroup substitutes;
        std::optional<std::string> defaultValue;
        std::optional<std::string> fixedValue;
        std::vector<const IdentityConstraint*> identityConstraints;
    };

    // Настройки загрузки схемы: откуда брать xs:include / xs:import
    // Schema loading settings: where to take xs:include / xs:import from
    struct SchemaLoadOptions
    {
        ResourceResolver* resolver = nullptr;
        std::string baseUri;
    };

    /*
    ** Schema — скомпилированная XML Schema (аналог XSModel / Grammar в Xerces).
    ** Неизменяема после загрузки и разделяется через shared_ptr: одну схему
    ** можно использовать для проверки множества документов, в том числе
    ** из разных потоков одновременно. Ошибки схемы — DomException(SCHEMA_ERROR).
    **
    ** Schema — a compiled XML Schema (the analog of XSModel / Grammar in Xerces).
    ** Immutable after loading and shared through shared_ptr: one schema
    ** can be used to validate many documents, including from different
    ** threads at the same time. Schema errors are DomException(SCHEMA_ERROR).
    */
    class Schema
    {
    public:
        Schema();

        // Загрузка из одного или нескольких документов схем (разные пространства имён)
        // Loading from one or several schema documents (different namespaces)
        static std::shared_ptr<const Schema> fromDocuments(const std::vector<const Document*>& documents,
            const SchemaLoadOptions& options = SchemaLoadOptions());

        // Загрузка из текста XSD
        // Loading from XSD text
        static std::shared_ptr<const Schema> fromString(std::string_view text,
            const SchemaLoadOptions& options = SchemaLoadOptions());

        // Загрузка из файла (xs:include рядом с файлом найдутся через options.resolver)
        // Loading from a file (xs:include next to the file is found through options.resolver)
        static std::shared_ptr<const Schema> fromFile(const std::filesystem::path& path,
            SchemaLoadOptions options = SchemaLoadOptions());

        // Глобальное объявление элемента или nullptr
        // A global element declaration or nullptr
        const ElementDecl* findElement(std::string_view namespaceUri, std::string_view localName) const;

        // Глобальный тип по имени (для xsi:type); found = false — тип не найден
        // A global type by name (for xsi:type); found = false — the type is not found
        TypeRef findType(std::string_view namespaceUri, std::string_view localName, bool& found) const;

    private:
        friend class detail::SchemaBuilder;

        using Key = std::pair<std::string, std::string>;

        std::deque<SimpleType> _simpleTypes;
        std::deque<ComplexType> _complexTypes;
        std::deque<ElementDecl> _elements;
        std::deque<IdentityConstraint> _identityConstraints;
        std::map<Key, const IdentityConstraint*> _identityByName;
        std::map<Key, const ElementDecl*> _globalElements;
        std::map<Key, TypeRef> _globalTypes;
    };
}

#endif
