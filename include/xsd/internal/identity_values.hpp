#ifndef XSD_IDENTITY_VALUES_HPP
#define XSD_IDENTITY_VALUES_HPP

#include "core/element.hpp"
#include "xsd/simple_type.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace beresta::xsd::detail
{
    /*
    ** NamespaceLookup — откуда брать URI префиксов для значений QName /
    ** NOTATION в ключах (паттерн Strategy). В потоке — стек объявлений
    ** открытых элементов (PrefixBindings), в дереве — подъём к корню
    ** (ElementNamespaces). В отличие от PrefixResolver, нужен сам URI, а не
    ** только факт объявления: "p:abc" и "q:abc" равны, если p и q объявлены
    ** одним URI.
    **
    ** NamespaceLookup — where to take the URIs of prefixes from for QName /
    ** NOTATION values in keys (the Strategy pattern). In a stream — the declaration stack
    ** of the open elements (PrefixBindings), in a tree — climbing to the root
    ** (ElementNamespaces). Unlike PrefixResolver, the URI itself is needed, not
    ** only the fact of a declaration: "p:abc" and "q:abc" are equal if p and q are declared
    ** with one URI.
    */
    class NamespaceLookup
    {
    public:
        virtual ~NamespaceLookup();

        // URI префикса ("" — пространство по умолчанию); nullopt — префикс не объявлен
        // The URI of a prefix ("" — the default namespace); nullopt — the prefix is not declared
        virtual std::optional<std::string> namespaceOf(std::string_view prefix) const = 0;
    };

    // Префиксы элемента дерева: поиск объявлений xmlns подъёмом к корню
    // The prefixes of a tree element: looking up xmlns declarations by climbing to the root
    class ElementNamespaces : public NamespaceLookup
    {
    public:
        explicit ElementNamespaces(const Element& element) noexcept;

        std::optional<std::string> namespaceOf(std::string_view prefix) const override;

    private:
        const Element& _element;
    };

    /*
    ** Каноническое значение поля ключа: как canonicalValue, но у QName и
    ** NOTATION (и списков из них) префикс заменён на URI — значения
    ** сравниваются по паре «пространство имён + локальное имя» (3.2.18).
    **
    ** The canonical value of a key field: as canonicalValue, but for QName and
    ** NOTATION (and lists of them) the prefix is replaced with the URI — the values
    ** are compared by the pair "namespace + local name" (3.2.18).
    */
    std::string identityValue(const SimpleType& type, std::string_view value, const NamespaceLookup& namespaces);
}

#endif
