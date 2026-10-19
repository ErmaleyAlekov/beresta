#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_functions.hpp"
#include "xpath/internal/xpath_typed.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xpath::detail
{
    /*
    ** sum() утверждений XSD: атомы — типизированные значения узлов. Если
    ** все они точные (xs:decimal, xs:integer, записи decimal), сумма точная —
    ** так sum(Tx/amount) с копейками не теряет ни одной; если среди них
    ** есть xs:double — сумма в double, как требует XPath 2.0. Даты не
    ** складываются (нужны длительности) — это ошибка.
    **
    ** sum() of XSD assertions: the atoms are the typed values of the nodes. If
    ** all of them are exact (xs:decimal, xs:integer, decimal notations), the sum is exact —
    ** this way sum(Tx/amount) with cents does not lose a single one; if there is
    ** an xs:double among them — the sum is in double, as XPath 2.0 requires. Dates are not
    ** added (durations are needed) — this is an error.
    */
    XPathValue typedSum(const XPathValue& value, const EvalContext& context)
    {
        std::vector<XPathValue> atoms = atomizeValue(value, context);
        utils::Decimal exact;
        double total = 0.0;
        bool allExact = true;

        for (const XPathValue& atom : atoms)
        {
            utils::Decimal part;
            if (isDateValue(atom))
                failEvaluation(i18n::format(i18n::Message::XPATH_DATE_ARITHMETIC));
            if (allExact && atom.getType() != ValueType::NUMBER && exactDecimal(atom, part))
                exact = utils::addDecimals(exact, part);
            else
                allExact = false;
            total += numericDouble(atom);
        }
        if (allExact)
            return XPathValue::fromDecimal(exact);
        return XPathValue::fromNumber(total);
    }

    /*
    ** floor / ceiling / round над точным числом в утверждениях XSD: результат
    ** остаётся xs:decimal. false — аргумент не точный (или режим XPath 1.0),
    ** тогда вызывающий считает в double.
    **
    ** floor / ceiling / round over an exact number in XSD assertions: the result
    ** stays xs:decimal. false — the argument is not exact (or the XPath 1.0 mode),
    ** then the caller computes in double.
    */
    bool exactRounding(const FunctionCall& call, DecimalRounding rounding, XPathValue& result)
    {
        XPathValue atom = XPathValue::fromBoolean(false);
        utils::Decimal exact;

        if (call.context.typed == nullptr || !singleAtom(argument(call, 0), call.context, atom))
            return false;
        if (atom.getType() == ValueType::NUMBER || !exactDecimal(atom, exact))
            return false;
        result = XPathValue::fromDecimal(rounding(exact));
        return true;
    }

    // exists(последовательность): есть ли хоть один член (XPath 2.0)
    // exists(sequence): whether there is at least one member (XPath 2.0)
    XPathValue functionExists(const FunctionCall& call)
    {
        XPathValue value = argument(call, 0);

        if (value.getType() != ValueType::NODE_SET)
            return XPathValue::fromBoolean(true);
        return XPathValue::fromBoolean(!value.getNodes().empty());
    }

    // empty(последовательность): пуста ли она (XPath 2.0)
    // empty(sequence): whether it is empty (XPath 2.0)
    XPathValue functionEmpty(const FunctionCall& call)
    {
        return XPathValue::fromBoolean(!functionExists(call).toBoolean());
    }
}
