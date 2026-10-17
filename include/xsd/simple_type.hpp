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
    // Пространство имён XML Schema
    inline constexpr std::string_view XSD_NAMESPACE_URI = "http://www.w3.org/2001/XMLSchema";

    // Пространство имён атрибутов экземпляра (xsi:type, xsi:nil)
    inline constexpr std::string_view XSI_NAMESPACE_URI = "http://www.w3.org/2001/XMLSchema-instance";

    // Примитивный тип — определяет лексическое пространство и порядок значений
    enum class Primitive
    {
        ANY_SIMPLE, STRING, BOOLEAN, DECIMAL, FLOAT, DOUBLE, DURATION, DATE_TIME, TIME, DATE,
        G_YEAR_MONTH, G_YEAR, G_MONTH_DAY, G_DAY, G_MONTH, HEX_BINARY, BASE64_BINARY,
        ANY_URI, QNAME, NOTATION_TYPE
    };

    // Дополнительное лексическое ограничение встроенных производных типов
    enum class LexicalForm
    {
        FREE, LANGUAGE, NAME, NCNAME, NMTOKEN, ID, IDREF, ENTITY, INTEGER
    };

    // Обработка пробелов перед проверкой значения
    enum class WhiteSpace
    {
        PRESERVE,   // как есть
        REPLACE,    // \t \n \r -> пробел
        COLLAPSE    // REPLACE + сжатие и обрезка пробелов
    };

    // Разновидность простого типа
    enum class Variety
    {
        ATOMIC, LIST, UNION
    };

    /*
    ** Фасеты — ограничения значения. При наследовании ограничением фасеты
    ** базового типа копируются и уточняются. Шаблоны хранятся группами:
    ** внутри группы (один шаг наследования) достаточно совпасть с любым,
    ** но значение обязано пройти КАЖДУЮ группу (все шаги наследования).
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
    const SimpleType* builtinType(std::string_view localName);

    /*
    ** Проверка значения простого типа. Возвращает nullopt, если значение
    ** корректно, иначе — текст ошибки. context — элемент, в котором
    ** разрешаются префиксы для значений типа QName.
    */
    std::optional<std::string> validateSimpleValue(const SimpleType& type, std::string_view value,
        const Element* context);

    // Значение после обработки пробелов по правилу whiteSpace
    std::string normalizeWhiteSpace(std::string_view value, WhiteSpace mode);

    // Равны ли два значения в пространстве значений типа (1.0 = 1 для decimal)
    bool valuesEqual(const SimpleType& type, std::string_view left, std::string_view right);
}

#endif
