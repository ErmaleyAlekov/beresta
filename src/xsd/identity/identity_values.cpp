#include "xsd/internal/identity_values.hpp"
#include "xsd/internal/datatype_checks.hpp"
#include "utils/qualified_name.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd::detail
{
    // Виртуальный деструктор интерфейса
    // The virtual destructor of the interface
    NamespaceLookup::~NamespaceLookup()
    {
    }

    // Проверка члена объединения использует те же объявления, что и ключ.
    // Union member validation uses the same namespace bindings as the key.
    bool NamespaceLookup::isBound(std::string_view prefix) const
    {
        return namespaceOf(prefix).has_value();
    }

    // Источник только запоминает элемент: поиск — при каждом запросе
    // The source only remembers the element: the lookup is done on each request
    ElementNamespaces::ElementNamespaces(const Element& element) noexcept
        : _element(element)
    {
    }

    // Объявления xmlns ищутся у элемента и его предков
    // The xmlns declarations are looked up at the element and its ancestors
    std::optional<std::string> ElementNamespaces::namespaceOf(std::string_view prefix) const
    {
        return _element.lookupNamespaceURI(prefix);
    }

    /*
    ** Одно имя QName / NOTATION: "{URI}локальное_имя" с префиксом
    ** примитивного типа, как у canonicalAtomic. Имя без префикса и без
    ** пространства по умолчанию — вне пространств имён (URI пуст).
    ** Необъявленный префикс (значение уже признано ошибочным) — прежнее
    ** сравнение по строке.
    **
    ** One QName / NOTATION name: "{URI}local_name" with the prefix of the
    ** primitive type, as in canonicalAtomic. A name without a prefix and without a
    ** default namespace — outside namespaces (the URI is empty).
    ** An undeclared prefix (the value has already been found invalid) — the former
    ** comparison by the string.
    */
    static std::string qualifiedKey(const SimpleType& type, std::string_view value, const NamespaceLookup& namespaces)
    {
        std::string normalized = normalizeWhiteSpace(value, WhiteSpace::COLLAPSE);
        utils::QualifiedNameParts parts = utils::splitQualifiedName(normalized);
        std::optional<std::string> namespaceUri = namespaces.namespaceOf(parts.prefix);

        if (!namespaceUri.has_value() && !parts.prefix.empty())
            return canonicalValue(type, value);
        std::string expanded = "{" + namespaceUri.value_or(std::string()) + "}";
        expanded.append(parts.localName.data(), parts.localName.size());
        return canonicalAtomic(type.primitive, expanded);
    }

    // Каждый элемент списка получает ключ своего фактического типа.
    // Each list item takes its key from its actual type.
    static std::string listKey(const SimpleType& itemType, std::string_view value, const NamespaceLookup& namespaces)
    {
        std::string normalized = normalizeWhiteSpace(value, WhiteSpace::COLLAPSE);
        std::string result;
        for (std::string_view item : utils::splitTokens(normalized))
        {
            if (!result.empty())
                result += ' ';
            result += identityValue(itemType, item, namespaces);
        }
        return result;
    }

    // Объединение использует первый допустимый член в порядке объявления.
    // A union uses the first valid member in declaration order.
    std::string identityValue(const SimpleType& type, std::string_view value, const NamespaceLookup& namespaces)
    {
        if (type.variety == Variety::UNION)
        {
            for (const SimpleType* member : type.memberTypes)
            {
                if (!validateSimpleValue(*member, value, namespaces))
                    return identityValue(*member, value, namespaces);
            }
            return normalizeWhiteSpace(value, type.facets.whiteSpace);
        }
        if (type.variety == Variety::LIST && type.itemType != nullptr)
            return listKey(*type.itemType, value, namespaces);
        if (type.primitive == Primitive::QNAME || type.primitive == Primitive::NOTATION_TYPE)
            return qualifiedKey(type, value, namespaces);
        return canonicalValue(type, value);
    }
}
