#ifndef XSD_PREFIX_RESOLVER_HPP
#define XSD_PREFIX_RESOLVER_HPP

#include "core/element.hpp"

#include <string_view>

namespace beresta::xsd
{
    /*
    ** PrefixResolver — откуда брать объявления префиксов для значений типа
    ** QName / NOTATION (паттерн Strategy). Значение "p:name" допустимо,
    ** только если префикс p объявлен в элементе, где стоит значение.
    ** В дереве префиксы ищутся подъёмом к корню (ElementPrefixResolver),
    ** в потоке SAX — по стеку объявлений xmlns открытых элементов: дерево
    ** для этого не нужно.
    **
    ** PrefixResolver — where to take prefix declarations from for values of the
    ** QName / NOTATION type (the Strategy pattern). The value "p:name" is allowed
    ** only if the prefix p is declared in the element where the value is.
    ** In a tree prefixes are looked up by climbing to the root (ElementPrefixResolver),
    ** in a SAX stream — by the stack of xmlns declarations of the open elements: no tree
    ** is needed for that.
    */
    class PrefixResolver
    {
    public:
        virtual ~PrefixResolver();

        // Объявлен ли префикс (xml и xmlns объявлены всегда)
        // Whether the prefix is declared (xml and xmlns are always declared)
        virtual bool isBound(std::string_view prefix) const = 0;
    };

    // Префиксы элемента дерева; nullptr — «элемента нет», и ни один префикс не объявлен
    // The prefixes of a tree element; nullptr means "no element", and no prefix is declared
    class ElementPrefixResolver : public PrefixResolver
    {
    public:
        explicit ElementPrefixResolver(const Element* element) noexcept;

        bool isBound(std::string_view prefix) const override;

    private:
        const Element* _element;
    };
}

#endif
