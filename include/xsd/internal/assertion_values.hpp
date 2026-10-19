#ifndef XSD_ASSERTION_VALUES_HPP
#define XSD_ASSERTION_VALUES_HPP

#include "xpath/internal/xpath_typed.hpp"
#include "xsd/simple_type.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace beresta::xsd::detail
{
    // ---- Типизированные значения (assertion_values.cpp) ----
    // ---- Typed values (assertion_values.cpp) ----

    /*
    ** Типизированное значение текста по простому типу — то, что видит
    ** утверждение: xs:decimal и производные (xs:integer, xs:long …) —
    ** точное DECIMAL, xs:float / xs:double — NUMBER, xs:boolean — BOOLEAN,
    ** xs:date / xs:dateTime — даты; объединение — по первому подходящему
    ** члену. Строки, списки, прочие типы и некорректный текст — STRING.
    **
    ** The typed value of a text by a simple type — what an assertion
    ** sees: xs:decimal and its derivatives (xs:integer, xs:long …) —
    ** an exact DECIMAL, xs:float / xs:double — NUMBER, xs:boolean — BOOLEAN,
    ** xs:date / xs:dateTime — dates; a union — by the first matching
    ** member. Strings, lists, other types and an invalid text — STRING.
    */
    xpath::XPathValue typedValueOf(const SimpleType& type, std::string_view text);

    // Строка -> xs:date / xs:dateTime (kind — DATE или DATE_TIME); false — не по лексике типа
    // A string -> xs:date / xs:dateTime (kind is DATE or DATE_TIME); false — not by the type lexis
    bool parseDateAtom(xpath::ValueType kind, std::string_view text, xpath::XPathValue& result);

    // ---- Фасет xs:assertion (assertion_facets.cpp) ----
    // ---- The xs:assertion facet (assertion_facets.cpp) ----

    /*
    ** Проверка утверждений owner.facets.assertions для значения value:
    ** $value — типизированное значение по valueType (сам owner или член
    ** объединения, подошедший значению). nullopt — все утверждения истинны.
    **
    ** Checks the owner.facets.assertions assertions for the value value:
    ** $value is the typed value by valueType (owner itself or the union
    ** member that matched the value). nullopt — all the assertions are true.
    */
    std::optional<std::string> checkAssertionFacets(const SimpleType& owner, const SimpleType& valueType,
        std::string_view value);

    /*
    ** Источник типов для фасетов: узлов у $value нет (контекстный узел —
    ** пустой служебный элемент), нужны только даты для xs:date(…).
    **
    ** The type source for facets: $value has no nodes (the context node is
    ** an empty service element), only dates for xs:date(…) are needed.
    */
    class FacetTypes : public xpath::detail::TypedValueSource
    {
    public:
        xpath::XPathValue atomize(const xpath::XPathItem& item) const override;
        bool castDate(xpath::ValueType type, std::string_view text, xpath::XPathValue& result) const override;
    };
}

#endif
