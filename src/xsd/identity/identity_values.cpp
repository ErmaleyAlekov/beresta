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

    /*
    ** Разрешение префиксов нужно только атомарным QName / NOTATION и
    ** спискам из них; у остальных типов (и объединений) — canonicalValue.
    **
    ** Resolving prefixes is needed only by atomic QName / NOTATION and
    ** lists of them; for the other types (and unions) — canonicalValue.
    */
    std::string identityValue(const SimpleType& type, std::string_view value, const NamespaceLookup& namespaces)
    {
        const SimpleType* atomic = &type;
        bool list = type.variety == Variety::LIST && type.itemType != nullptr;

        if (list)
            atomic = type.itemType;
        bool qualified = atomic->primitive == Primitive::QNAME || atomic->primitive == Primitive::NOTATION_TYPE;
        if (atomic->variety != Variety::ATOMIC || !qualified)
            return canonicalValue(type, value);
        if (!list)
            return qualifiedKey(type, value, namespaces);
        std::string normalized = normalizeWhiteSpace(value, WhiteSpace::COLLAPSE);
        std::string result;
        for (std::string_view item : utils::splitTokens(normalized))
        {
            if (!result.empty())
                result += ' ';
            result += qualifiedKey(*atomic, item, namespaces);
        }
        return result;
    }
}
