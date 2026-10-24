#ifndef XSD_FACET_RULES_HPP
#define XSD_FACET_RULES_HPP

#include "xsd/simple_type.hpp"

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::xsd::detail
{
    /*
    ** Один шаг ограничения простого типа (xs:restriction) для проверки
    ** правил фасетов (XSD 1.0, часть 2, §4.3): базовый тип, фасеты базы до
    ** шага, фасеты, названные в самом шаге (кроме pattern, enumeration и
    ** утверждений), и сырые значения перечисления шага. context — элемент
    ** xs:restriction: по нему разрешаются префиксы значений типа QName.
    ** Объект параметров (DTO) вместо длинного списка аргументов.
    **
    ** One restriction step of a simple type (xs:restriction) for checking the
    ** facet rules (XSD 1.0, part 2, §4.3): the base type, the base's facets before
    ** the step, the facets named in the step itself (except pattern, enumeration and
    ** assertions), and the raw enumeration values of the step. context is the
    ** xs:restriction element: the prefixes of QName values are resolved by it.
    ** A parameter object (DTO) instead of a long argument list.
    */
    struct FacetStep
    {
        const SimpleType* base = nullptr;
        Facets inherited;
        std::map<std::string, std::string> named;
        std::vector<std::string> enumeration;
        const Element* context = nullptr;
    };

    // ---- Значения фасетов (facet_values.cpp) ----
    // ---- Facet values (facet_values.cpp) ----

    // Числовой, граничный фасет или whiteSpace в facets; false — фасет неизвестен или значение некорректно
    // A number facet, a bound facet or whiteSpace into facets; false — the facet is unknown or the value is invalid
    bool applySimpleFacet(Facets& facets, std::string_view kind, const std::string& value);

    // ---- Виды фасетов (facet_rules_kinds.cpp) ----
    // ---- Facet kinds (facet_rules_kinds.cpp) ----

    // Применим ли фасет к ограничению базового типа (разновидность и примитив, часть 2, §4.1.5)
    // Whether the facet is applicable to a restriction of the base type (the variety and the primitive, part 2, §4.1.5)
    bool facetApplies(const SimpleType& base, std::string_view facet);

    // min/max Inclusive/Exclusive
    bool isBoundFacet(std::string_view facet) noexcept;

    // length, minLength, maxLength
    bool isLengthFacet(std::string_view facet) noexcept;

    // ---- Проверка шага (facet_rules_check.cpp) ----
    // ---- Checking a step (facet_rules_check.cpp) ----

    /*
    ** Все правила фасетов одного шага; type — тип после шага (фасеты базы,
    ** уточнённые шагом). nullopt — шаг корректен, иначе текст ошибки.
    **
    ** All facet rules of one step; type is the type after the step (the base's
    ** facets refined by the step). nullopt — the step is valid, otherwise the error text.
    */
    std::optional<std::string> checkFacetStep(const FacetStep& step, const SimpleType& type);

    // ---- Длины и цифры (facet_rules_lengths.cpp) ----
    // ---- Lengths and digits (facet_rules_lengths.cpp) ----
    std::optional<std::string> checkLengthFacets(const FacetStep& step, const Facets& merged);
    std::optional<std::string> checkDigitFacets(const FacetStep& step, const Facets& merged);

    // ---- Границы (facet_rules_bounds.cpp) ----
    // ---- Bounds (facet_rules_bounds.cpp) ----
    std::optional<std::string> checkBoundValues(const FacetStep& step);
    std::optional<std::string> checkBoundRestriction(const FacetStep& step);
    std::optional<std::string> checkBoundOrder(const Facets& merged, Primitive primitive);
}

#endif
