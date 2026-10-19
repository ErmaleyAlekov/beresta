#include "xsd/internal/assertion_values.hpp"
#include "xsd/assertion.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "xpath/internal/xpath_assertion.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Каждое утверждение фасета вычисляется с $value; первое ложное (или
    ** упавшее с ошибкой вычисления) — текст ошибки значения. Контекстного
    ** узла у фасета нет: вместо него — пустой служебный элемент, общий и
    ** неизменяемый (безопасен при проверке из многих потоков).
    **
    ** Each assertion of the facet is evaluated with $value; the first false one (or
    ** the one failed with an evaluation error) is the text of the value error. The facet
    ** has no context node: instead of it — an empty service element, shared and
    ** immutable (safe when validating from many threads).
    */
    std::optional<std::string> checkAssertionFacets(const SimpleType& owner, const SimpleType& valueType,
        std::string_view value)
    {
        static const FacetTypes types{};
        static const Element context("value");
        xpath::XPathValue typed = typedValueOf(valueType, value);

        for (const std::shared_ptr<const Assertion>& assertion : owner.facets.assertions)
        {
            const std::string& test = assertion->test.getSource();
            try
            {
                if (!xpath::detail::ExpressionAccess::evaluateAssertion(assertion->test, context,
                    assertion->context, types, &typed))
                    return i18n::format(i18n::Message::XSD_ASSERTION_FACET_FAILED, {value, test});
            }
            catch (const DomException& error)
            {
                return i18n::format(i18n::Message::XSD_ASSERTION_ERROR, {test, error.getMessage()});
            }
        }
        return std::nullopt;
    }

    // У служебного элемента-контекста типа нет: строковое значение
    // The service context element has no type: the string value
    xpath::XPathValue FacetTypes::atomize(const xpath::XPathItem& item) const
    {
        return xpath::XPathValue::fromString(xpath::stringValue(item));
    }

    // Даты для конструкторов xs:date / xs:dateTime и сравнений со строками
    // Dates for the xs:date / xs:dateTime constructors and comparisons with strings
    bool FacetTypes::castDate(xpath::ValueType type, std::string_view text, xpath::XPathValue& result) const
    {
        return parseDateAtom(type, text, result);
    }
}
