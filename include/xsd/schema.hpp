#ifndef XSD_SCHEMA_HPP
#define XSD_SCHEMA_HPP

#include "content/particle.hpp"
#include "core/document.hpp"
#include "parser/resource_resolver.hpp"
#include "xsd/simple_type.hpp"

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

    namespace detail
    {
        class SchemaBuilder;
    }

    // Ссылка на тип: простой, составной или (оба nullptr) xs:anyType — «что угодно»
    struct TypeRef
    {
        const SimpleType* simple = nullptr;
        const ComplexType* complex = nullptr;
    };

    // Режим проверки содержимого, допущенного шаблоном xs:any / xs:anyAttribute
    enum class ProcessContents
    {
        STRICT_CHECK,   // объявление обязано найтись и проверяется
        LAX_CHECK,      // проверяется, если объявление найдено
        SKIP_CHECK      // не проверяется
    };

    // Вид содержимого составного типа
    enum class ContentType
    {
        EMPTY_CONTENT,  // ни детей, ни текста
        SIMPLE_CONTENT, // только текст простого типа
        ELEMENT_ONLY,   // только элементы (пробелы между ними допустимы)
        MIXED_CONTENT   // элементы вперемешку с текстом
    };

    // Использование атрибута в составном типе
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

    // Составной тип: модель содержимого и атрибуты
    struct ComplexType
    {
        std::string name;
        std::string namespaceUri;
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

    // Объявление элемента
    struct ElementDecl
    {
        std::string name;
        std::string namespaceUri;
        TypeRef type;
        bool nillable = false;
        bool isAbstract = false;
        std::optional<std::string> defaultValue;
        std::optional<std::string> fixedValue;
    };

    // Настройки загрузки схемы: откуда брать xs:include / xs:import
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
    */
    class Schema
    {
    public:
        Schema();

        // Загрузка из одного или нескольких документов схем (разные пространства имён)
        static std::shared_ptr<const Schema> fromDocuments(const std::vector<const Document*>& documents,
            const SchemaLoadOptions& options = SchemaLoadOptions());

        // Загрузка из текста XSD
        static std::shared_ptr<const Schema> fromString(std::string_view text,
            const SchemaLoadOptions& options = SchemaLoadOptions());

        // Загрузка из файла (xs:include рядом с файлом найдутся через options.resolver)
        static std::shared_ptr<const Schema> fromFile(const std::filesystem::path& path,
            SchemaLoadOptions options = SchemaLoadOptions());

        // Глобальное объявление элемента или nullptr
        const ElementDecl* findElement(std::string_view namespaceUri, std::string_view localName) const;

        // Глобальный тип по имени (для xsi:type); found = false — тип не найден
        TypeRef findType(std::string_view namespaceUri, std::string_view localName, bool& found) const;

    private:
        friend class detail::SchemaBuilder;

        using Key = std::pair<std::string, std::string>;

        std::deque<SimpleType> _simpleTypes;
        std::deque<ComplexType> _complexTypes;
        std::deque<ElementDecl> _elements;
        std::map<Key, const ElementDecl*> _globalElements;
        std::map<Key, TypeRef> _globalTypes;
    };
}

#endif
