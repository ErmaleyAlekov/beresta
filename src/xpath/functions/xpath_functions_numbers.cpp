#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_functions.hpp"

#include <cmath>

namespace beresta::xpath::detail
{
    // number(значение?): приведение к числу
    XPathValue functionNumber(const FunctionCall& call)
    {
        return XPathValue::fromNumber(numberArgument(call, 0));
    }

    // sum(множество): сумма чисел из строковых значений узлов
    XPathValue functionSum(const FunctionCall& call)
    {
        XPathValue value = argument(call, 0);
        double total = 0.0;

        if (value.getType() != ValueType::NODE_SET)
            failEvaluation("sum() ожидает множество узлов");
        for (const XPathItem& item : value.getNodes())
            total += stringToNumber(stringValue(item));
        return XPathValue::fromNumber(total);
    }

    // floor(число): округление вниз
    XPathValue functionFloor(const FunctionCall& call)
    {
        return XPathValue::fromNumber(std::floor(numberArgument(call, 0)));
    }

    // ceiling(число): округление вверх
    XPathValue functionCeiling(const FunctionCall& call)
    {
        return XPathValue::fromNumber(std::ceil(numberArgument(call, 0)));
    }

    // round(число): к ближайшему целому, половина — вверх (round(-2.5) = -2)
    XPathValue functionRound(const FunctionCall& call)
    {
        double value = numberArgument(call, 0);

        if (std::isnan(value) || std::isinf(value))
            return XPathValue::fromNumber(value);
        return XPathValue::fromNumber(std::floor(value + 0.5));
    }
}
