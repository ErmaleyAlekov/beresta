#include "xsd/internal/schema_builder.hpp"
#include "xsd/internal/facet_rules.hpp"
#include "i18n/i18n.hpp"

#include <memory>

namespace beresta::xsd::detail
{
    // Дети xs:restriction, которые не являются фасетами (тип, атрибуты, утверждения)
    // The children of xs:restriction that are not facets (a type, attributes, assertions)
    static bool isFacetlessChild(std::string_view kind) noexcept
    {
        return kind == "simpleType" || kind == "attribute" || kind == "attributeGroup"
            || kind == "anyAttribute" || kind == "assert" || kind == "assertion";
    }

    /*
    ** Фасеты одного шага ограничения. Шаблоны шага образуют одну группу
    ** (достаточно совпасть с любым), перечисление шага заменяет базовое,
    ** утверждения xs:assertion добавляются к унаследованным. xs:assert
    ** внутри xs:simpleContent / xs:restriction — не фасет, его читает тип.
    ** После чтения шаг проверяется по правилам фасетов (checkFacetStep):
    ** применимость, допустимость значений для базового типа, сочетания и
    ** то, что ограничение не ослабляет базовое.
    **
    ** The facets of one restriction step. The patterns of a step form one group
    ** (matching any is enough), the enumeration of a step replaces the base one,
    ** the xs:assertion assertions are added to the inherited ones. xs:assert
    ** inside xs:simpleContent / xs:restriction is not a facet, the type reads it.
    ** After reading, the step is checked by the facet rules (checkFacetStep):
    ** applicability, the admissibility of the values for the base type, combinations and
    ** that the restriction does not weaken the base one.
    */
    void SchemaBuilder::applyFacets(SimpleType& type, const Element& restriction, const SchemaSource& source)
    {
        std::vector<std::shared_ptr<const Regex>> patterns;
        FacetStep step{type.baseType, type.facets, {}, {}, &restriction};

        for (const Element* facet : schemaChildren(restriction))
        {
            std::string_view kind = facet->getLocalName();
            std::string raw = facet->getAttribute("value").value_or("");
            if (isFacetlessChild(kind))
                continue;
            if (kind == "pattern")
                patterns.push_back(std::make_shared<Regex>(raw));
            else if (kind == "enumeration")
                step.enumeration.push_back(raw);
            else
                applyNamedFacet(type, step, *facet);
        }
        if (!patterns.empty())
            type.facets.patterns.push_back(patterns);
        if (!step.enumeration.empty())
            type.facets.enumeration.clear();
        for (const std::string& value : step.enumeration)
            type.facets.enumeration.push_back(normalizeWhiteSpace(value, type.facets.whiteSpace));
        std::optional<std::string> error = checkFacetStep(step, type);
        if (error.has_value())
            fail(restriction, error.value());
        readAssertionFacets(type, restriction, source);
    }

    // Фасет с одним значением (не pattern / enumeration): повтор в шаге и некорректное значение — ошибки
    // A single-valued facet (not pattern / enumeration): a repetition in the step and an invalid value are errors
    void SchemaBuilder::applyNamedFacet(SimpleType& type, FacetStep& step, const Element& facet) const
    {
        std::string kind(facet.getLocalName());
        std::string value = normalizeWhiteSpace(facet.getAttribute("value").value_or(""), WhiteSpace::COLLAPSE);

        if (!step.named.emplace(kind, value).second)
            fail(facet, i18n::format(i18n::Message::SCHEMA_FACET_DUPLICATE, {kind}));
        if (!applySimpleFacet(type.facets, kind, value))
            fail(facet, i18n::format(i18n::Message::SCHEMA_FACET_INVALID, {kind}));
    }
}
