#include "xpath/internal/xpath_eval.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Унарный минус владеет операндом
    NegateExpr::NegateExpr(ExprPtr operand) noexcept
        : _operand(std::move(operand))
    {
    }

    // Отрицание числового значения операнда
    XPathValue NegateExpr::evaluate(const EvalContext& context) const
    {
        return XPathValue::fromNumber(-_operand->evaluate(context).toNumber());
    }

    // Константа хранит готовое значение
    ConstantExpr::ConstantExpr(XPathValue value)
        : _value(std::move(value))
    {
    }

    // Значение константы не зависит от контекста
    XPathValue ConstantExpr::evaluate(const EvalContext& context) const
    {
        (void)context;
        return _value;
    }

    // Переменная хранит только своё имя
    VariableExpr::VariableExpr(std::string name)
        : _name(std::move(name))
    {
    }
}
