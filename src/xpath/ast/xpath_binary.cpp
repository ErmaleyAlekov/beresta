#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_typed.hpp"
#include "i18n/i18n.hpp"

#include <cmath>
#include <utility>

namespace beresta::xpath::detail
{
    // Виртуальный деструктор базового узла дерева разбора
    // The virtual destructor of the base parse tree node
    Expr::~Expr()
    {
    }

    // Бинарное выражение владеет обоими операндами
    // A binary expression owns both operands
    BinaryExpr::BinaryExpr(BinaryOperator op, ExprPtr left, ExprPtr right) noexcept
        : _operator(op), _left(std::move(left)), _right(std::move(right))
    {
    }

    /*
    ** Деление по IEEE 754 без опоры на неопределённое поведение:
    ** x/0 — бесконечность со знаком, 0/0 и NaN/0 — NaN (как требует XPath).
    **
    ** Division by IEEE 754 without relying on undefined behavior:
    ** x/0 is a signed infinity, 0/0 and NaN/0 are NaN (as XPath requires).
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
    // Arithmetic on numbers (mod is the remainder with the sign of the dividend, like fmod)
    double numberArithmetic(BinaryOperator op, double left, double right) noexcept
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
    ** Операция над готовым левым значением: or / and — с коротким замыканием
    ** (правый операнд не вычисляется, если исход уже ясен); сравнения — по
    ** правилам XPath для множеств; '|' — объединение множеств в порядке
    ** документа. В утверждениях XSD (typed) сравнения и арифметика — типизированные.
    **
    ** The operation over a ready left value: or / and with short-circuiting
    ** (the right operand is not evaluated when the outcome is already known); comparisons
    ** follow the XPath rules for sets; '|' is the union of sets in document
    ** order. In XSD assertions (typed) comparisons and arithmetic are typed.
    */
    XPathValue BinaryExpr::combine(XPathValue left, const EvalContext& context) const
    {
        if (_operator == BinaryOperator::OR_OP)
            return XPathValue::fromBoolean(left.toBoolean() || _right->evaluate(context).toBoolean());
        if (_operator == BinaryOperator::AND_OP)
            return XPathValue::fromBoolean(left.toBoolean() && _right->evaluate(context).toBoolean());
        XPathValue right = _right->evaluate(context);
        if (_operator == BinaryOperator::UNION_OP)
        {
            if (left.getType() != ValueType::NODE_SET || right.getType() != ValueType::NODE_SET)
                failEvaluation(i18n::format(i18n::Message::XPATH_UNION_NOT_NODESET));
            std::vector<XPathItem> merged = left.getNodes();
            merged.insert(merged.end(), right.getNodes().begin(), right.getNodes().end());
            sortDocumentOrder(merged);
            return XPathValue::fromNodeSet(std::move(merged));
        }
        if (context.typed != nullptr)
            return typedBinary(_operator, left, right, context);
        if (_operator >= BinaryOperator::EQUAL_OP && _operator <= BinaryOperator::GREATER_EQUAL_OP)
            return XPathValue::fromBoolean(compareValues(_operator, left, right));
        return XPathValue::fromNumber(numberArithmetic(_operator, left.toNumber(), right.toNumber()));
    }
}
