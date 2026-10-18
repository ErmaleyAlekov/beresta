#ifndef XSD_SIMPLE_TYPE_HPP
#define XSD_SIMPLE_TYPE_HPP

#include "core/element.hpp"
#include "xsd/regex/xsd_regex.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::xsd
{
    class PrefixResolver;

    // Пространство имён XML Schema
    // The XML Schema namespace
    inline constexpr std::string_view XSD_NAMESPACE_URI = "http://www.w3.org/2001/XMLSchema";

    // Пространство имён атрибутов экземпляра (xsi:type, xsi:nil)
    // The namespace of instance attributes (xsi:type, xsi:nil)
    inline constexpr std::string_view XSI_NAMESPACE_URI = "http://www.w3.org/2001/XMLSchema-instance";

    // Примитивный тип — определяет лексическое пространство и порядок значений
    // A primitive type — defines the lexical space and the order of values
    enum class Primitive
    {
        ANY_SIMPLE, STRING, BOOLEAN, DECIMAL, FLOAT, DOUBLE, DURATION, DATE_TIME, TIME, DATE,
        G_YEAR_MONTH, G_YEAR, G_MONTH_DAY, G_DAY, G_MONTH, HEX_BINARY, BASE64_BINARY,
        ANY_URI, QNAME, NOTATION_TYPE
    };

    // Дополнительное лексическое ограничение встроенных производных типов
    // An additional lexical restriction of built-in derived types
    enum class LexicalForm
    {
        FREE, LANGUAGE, NAME, NCNAME, NMTOKEN, ID, IDREF, ENTITY, INTEGER
    };

    // Обработка пробелов перед проверкой значения
    // Whitespace processing before the value check
    enum class WhiteSpace
    {
        PRESERVE,   // как есть / as is
        REPLACE,    // \t \n \r -> пробел / \t \n \r -> a space
        COLLAPSE    // REPLACE + сжатие и обрезка пробелов / REPLACE + collapsing and trimming spaces
    };

    // Разновидность простого типа
    // The variety of a simple type
    enum class Variety
    {
        ATOMIC, LIST, UNION
    };

    /*
    ** Фасеты — ограничения значения. При наследовании ограничением фасеты
    ** базового типа копируются и уточняются. Шаблоны хранятся группами:
    ** внутри группы (один шаг наследования) достаточно совпасть с любым,
    ** но значение обязано пройти КАЖДУЮ группу (все шаги наследования).
    **
    ** Facets are value restrictions. On derivation by restriction the facets
    ** of the base type are copied and refined. Patterns are stored in groups:
    ** inside a group (one derivation step) matching any one is enough,
    ** but the value must pass EVERY group (all derivation steps).
    */
    struct Facets
    {
        std::optional<std::size_t> length;
        std::optional<std::size_t> minLength;
        std::optional<std::size_t> maxLength;
        std::optional<std::size_t> totalDigits;
        std::optional<std::size_t> fractionDigits;
        std::vector<std::string> enumeration;
        std::vector<std::vector<std::shared_ptr<const Regex>>> patterns;
        std::optional<std::string> minInclusive;
        std::optional<std::string> maxInclusive;
        std::optional<std::string> minExclusive;
        std::optional<std::string> maxExclusive;
        WhiteSpace whiteSpace = WhiteSpace::PRESERVE;
    };

    // Простой тип XML Schema: встроенный или объявленный в схеме
    // An XML Schema simple type: built-in or declared in the schema
    struct SimpleType
    {
        std::string name;
        std::string namespaceUri;
        Variety variety = Variety::ATOMIC;
        Primitive primitive = Primitive::STRING;
        LexicalForm form = LexicalForm::FREE;
        const SimpleType* itemType = nullptr;
        std::vector<const SimpleType*> memberTypes;
        Facets facets;
    };

    // Встроенный тип по локальному имени в пространстве XSD (nullptr — неизвестен)
    // A built-in type by local name in the XSD namespace (nullptr — unknown)
    const SimpleType* builtinType(std::string_view localName);

    /*
    ** Проверка значения простого типа. Возвращает nullopt, если значение
    ** корректно, иначе — текст ошибки. context — элемент, в котором
    ** разрешаются префиксы для значений типа QName.
    **
    ** Checks a value of the simple type. Returns nullopt if the value
    ** is valid, otherwise the error text. context is the element in which
    ** prefixes are resolved for QName values.
    */
    std::optional<std::string> validateSimpleValue(const SimpleType& type, std::string_view value,
        const Element* context);

    // То же, но префиксы QName берутся из prefixes (например, из стека объявлений потока SAX)
    // The same, but QName prefixes are taken from prefixes (for example, from the declaration stack of a SAX stream)
    std::optional<std::string> validateSimpleValue(const SimpleType& type, std::string_view value,
        const PrefixResolver& prefixes);

    // Значение после обработки пробелов по правилу whiteSpace
    // The value after whitespace processing by the whiteSpace rule
    std::string normalizeWhiteSpace(std::string_view value, WhiteSpace mode);

    // Равны ли два значения в пространстве значений типа (1.0 = 1 для decimal)
    // Whether two values are equal in the value space of the type (1.0 = 1 for decimal)
    bool valuesEqual(const SimpleType& type, std::string_view left, std::string_view right);

    /*
    ** Каноническое представление значения: равные в пространстве значений
    ** типа значения дают одинаковые строки ("1.0" и "001" для decimal,
    ** "1" и "true" для boolean, даты с разными часовыми поясами — по UTC).
    ** Значения разных примитивных типов никогда не совпадают. Используется
    ** как ключ в таблицах xs:key / xs:unique (поиск за O(log n)).
    **
    ** The canonical representation of a value: values equal in the value space
    ** of the type give the same strings ("1.0" and "001" for decimal,
    ** "1" and "true" for boolean, dates with different time zones — by UTC).
    ** Values of different primitive types never match. Used
    ** as the key in the xs:key / xs:unique tables (lookup in O(log n)).
    */
    std::string canonicalValue(const SimpleType& type, std::string_view value);
}

#endif
