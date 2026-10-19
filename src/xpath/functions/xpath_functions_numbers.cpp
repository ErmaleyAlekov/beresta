#include "xpath/internal/xpath_eval.hpp"
#include "i18n/i18n.hpp"
#include "xpath/internal/xpath_functions.hpp"

#include <cmath>

namespace beresta::xpath::detail
{
    // number(значение?): приведение к числу
    // number(value?): conversion to a number
    XPathValue functionNumber(const FunctionCall& call)
    {
        return XPathValue::fromNumber(numberArgument(call, 0));
    }

    /*
    ** sum(множество): сумма чисел из строковых значений узлов. В утверждениях
    ** XSD — по типизированным значениям, точно для xs:decimal (typedSum).
    **
    ** sum(set): the sum of the numbers from the string values of the nodes. In XSD
    ** assertions — over the typed values, exactly for xs:decimal (typedSum).
    */
    XPathValue functionSum(const FunctionCall& call)
    {
        XPathValue value = argument(call, 0);
        double total = 0.0;

        if (call.context.typed != nullptr)
            return typedSum(value, call.context);
        if (value.getType() != ValueType::NODE_SET)
            failEvaluation(i18n::format(i18n::Message::XPATH_SUM_NOT_NODESET));
        for (const XPathItem& item : value.getNodes())
            total += stringToNumber(stringValue(item));
        return XPathValue::fromNumber(total);
    }

    // floor(число): округление вниз (точное число остаётся точным)
    // floor(number): rounding down (an exact number stays exact)
    XPathValue functionFloor(const FunctionCall& call)
    {
        XPathValue exact = XPathValue::fromBoolean(false);

        if (exactRounding(call, utils::floorDecimal, exact))
            return exact;
        return XPathValue::fromNumber(std::floor(numberArgument(call, 0)));
    }

    // ceiling(число): округление вверх (точное число остаётся точным)
    // ceiling(number): rounding up (an exact number stays exact)
    XPathValue functionCeiling(const FunctionCall& call)
    {
        XPathValue exact = XPathValue::fromBoolean(false);

        if (exactRounding(call, utils::ceilingDecimal, exact))
            return exact;
        return XPathValue::fromNumber(std::ceil(numberArgument(call, 0)));
    }

    // round(число): к ближайшему целому, половина — вверх (round(-2.5) = -2)
    // round(number): to the nearest integer, a half goes up (round(-2.5) = -2)
    XPathValue functionRound(const FunctionCall& call)
    {
        XPathValue exact = XPathValue::fromBoolean(false);
        double value = 0.0;

        if (exactRounding(call, utils::roundDecimal, exact))
            return exact;
        value = numberArgument(call, 0);
        if (std::isnan(value) || std::isinf(value))
            return XPathValue::fromNumber(value);
        return XPathValue::fromNumber(std::floor(value + 0.5));
    }
}
