#include "xpath/internal/xpath_eval.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Путь: начальное выражение (или корень, или текущий узел) и шаги
    PathExpr::PathExpr(ExprPtr start, bool absolute, std::vector<Step> steps)
        : _start(std::move(start)), _absolute(absolute), _steps(std::move(steps))
    {
    }

    // Начальное множество, затем последовательное применение шагов
    XPathValue PathExpr::evaluate(const EvalContext& context) const
    {
        std::vector<XPathItem> current;

        if (_start)
        {
            XPathValue value = _start->evaluate(context);
            if (value.getType() != ValueType::NODE_SET)
                failEvaluation("путь можно продолжить только от множества узлов");
            current = value.getNodes();
        }
        else if (_absolute)
            current.push_back(rootOf(context.item));
        else
            current.push_back(context.item);
        for (const Step& step : _steps)
            current = applyStep(step, current, context.user);
        return XPathValue::fromNodeSet(std::move(current));
    }

    // Ошибки вычисления используют тот же код, что и ошибки разбора выражения
    void failEvaluation(const std::string& message)
    {
        throw DomException(DomErrorCode::XPATH_ERROR, "XPath: " + message);
    }
}
