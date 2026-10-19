#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_typed.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Унарный минус владеет операндом
    // A unary minus owns its operand
    NegateExpr::NegateExpr(ExprPtr operand) noexcept
        : _operand(std::move(operand))
    {
    }

    // Отрицание числового значения операнда (в утверждениях XSD — точное для decimal)
    // The negation of the numeric value of the operand (in XSD assertions — exact for a decimal)
    XPathValue NegateExpr::evaluate(const EvalContext& context) const
    {
        if (context.typed != nullptr)
            return typedNegate(_operand->evaluate(context), context);
        return XPathValue::fromNumber(-_operand->evaluate(context).toNumber());
    }

    // Константа хранит готовое значение
    // A constant keeps a ready value
    ConstantExpr::ConstantExpr(XPathValue value)
        : _value(std::move(value))
    {
    }

    // Значение константы не зависит от контекста
    // The constant value does not depend on the context
    XPathValue ConstantExpr::evaluate(const EvalContext& context) const
    {
        (void)context;
        return _value;
    }

    // Переменная хранит только своё имя
    // A variable keeps only its name
    VariableExpr::VariableExpr(std::string name)
        : _name(std::move(name))
    {
    }
}
