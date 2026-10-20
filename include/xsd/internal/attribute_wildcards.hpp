#ifndef XSD_ATTRIBUTE_WILDCARDS_HPP
#define XSD_ATTRIBUTE_WILDCARDS_HPP

#include "xsd/schema.hpp"

#include <optional>
#include <string_view>

namespace beresta::xsd::detail
{
    // Допускает ли шаблон атрибут из пространства имён namespaceUri (пусто — без пространства)
    // Whether the wildcard admits an attribute from the namespaceUri namespace (empty — no namespace)
    bool wildcardAllows(const AttributeWildcard& wildcard, std::string_view namespaceUri) noexcept;

    /*
    ** Пересечение ограничений пространств имён (3.10.6, Attribute Wildcard
    ** Intersection, 2-я редакция); processContents берётся у left.
    ** nullopt — пересечение невыразимо (##other двух разных пространств).
    **
    ** The intersection of namespace constraints (3.10.6, Attribute Wildcard
    ** Intersection, 2nd edition); processContents is taken from left.
    ** nullopt — the intersection is not expressible (##other of two different namespaces).
    */
    std::optional<AttributeWildcard> intersectWildcards(const AttributeWildcard& left,
        const AttributeWildcard& right);

    /*
    ** Объединение ограничений (3.10.6, Attribute Wildcard Union);
    ** processContents берётся у left. nullopt — объединение невыразимо
    ** (##other пространства x и перечень, где есть «без пространства», но нет x).
    **
    ** The union of constraints (3.10.6, Attribute Wildcard Union);
    ** processContents is taken from left. nullopt — the union is not expressible
    ** (##other of the namespace x and a list with "no namespace" but without x).
    */
    std::optional<AttributeWildcard> uniteWildcards(const AttributeWildcard& left,
        const AttributeWildcard& right);
}

#endif
