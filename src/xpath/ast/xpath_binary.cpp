#include "xpath/internal/xpath_eval.hpp"

#include <cmath>
#include <utility>

namespace beresta::xpath::detail
{
    // Виртуальный деструктор базового узла дерева разбора
    Expr::~Expr()
    {
    }

    // Бинарное выражение владеет обоими операндами
    BinaryExpr::BinaryExpr(BinaryOperator op, ExprPtr left, ExprPtr right) noexcept
        : _operator(op), _left(std::move(left)), _right(std::move(right))
    {
    }

    /*
    ** Деление по IEEE 754 без опоры на неопределённое поведение:
    ** x/0 — бесконечность со знаком, 0/0 и NaN/0 — NaN (как требует XPath).
    */
    static double divide(double left, double right) noexcept
    {
        if (right != 0.0)
            return left / right;
        if (left == 0.0 || std::isnan(left))
            return std::nan("");
        if (std::signbit(left) != std::signbit(right))
            return -HUGE_VAL;
        return HUGE_VAL;
    }

    // Арифметика над числами (mod — остаток с знаком делимого, как fmod)
    static double arithmetic(BinaryOperator op, double left, double right) noexcept
    {
        if (op == BinaryOperator::PLUS_OP)
            return left + right;
        if (op == BinaryOperator::MINUS_OP)
            return left - right;
        if (op == BinaryOperator::MULTIPLY_OP)
            return left * right;
        if (op == BinaryOperator::DIVIDE_OP)
            return divide(left, right);
        if (right == 0.0)
            return std::nan("");
        return std::fmod(left, right);
    }

    /*
    ** or / and вычисляются с коротким замыканием; сравнения — по правилам
    ** XPath для множеств; '|' — объединение множеств в порядке документа.
    */
    XPathValue BinaryExpr::evaluate(const EvalContext& context) const
    {
        if (_operator == BinaryOperator::OR_OP)
            return XPathValue::fromBoolean(_left->evaluate(context).toBoolean()
                || _right->evaluate(context).toBoolean());
        if (_operator == BinaryOperator::AND_OP)
            return XPathValue::fromBoolean(_left->evaluate(context).toBoolean()
                && _right->evaluate(context).toBoolean());
        XPathValue left = _left->evaluate(context);
        XPathValue right = _right->evaluate(context);
        if (_operator == BinaryOperator::UNION_OP)
        {
            if (left.getType() != ValueType::NODE_SET || right.getType() != ValueType::NODE_SET)
                failEvaluation("операция '|' применима только к множествам узлов");
            std::vector<XPathItem> merged = left.getNodes();
            merged.insert(merged.end(), right.getNodes().begin(), right.getNodes().end());
            sortDocumentOrder(merged);
            return XPathValue::fromNodeSet(std::move(merged));
        }
        if (_operator >= BinaryOperator::EQUAL_OP && _operator <= BinaryOperator::GREATER_EQUAL_OP)
            return XPathValue::fromBoolean(compareValues(_operator, left, right));
        return XPathValue::fromNumber(arithmetic(_operator, left.toNumber(), right.toNumber()));
    }
}
