#include "xpath/internal/xpath_typed.hpp"
#include "xpath/internal/xpath_eval.hpp"
#include "i18n/i18n.hpp"

#include <cmath>

namespace beresta::xpath::detail
{
    /*
    ** Точная арифметика xs:decimal. Деление — с DECIMAL_DIVISION_SCALE
    ** цифрами дробной части; деление и mod на ноль — ошибка FOAR0001
    ** (в XPath 1.0 было бы ±Infinity, но у decimal бесконечности нет).
    **
    ** Exact xs:decimal arithmetic. Division — with DECIMAL_DIVISION_SCALE
    ** fraction digits; division and mod by zero is the FOAR0001 error
    ** (XPath 1.0 would give ±Infinity, but decimal has no infinity).
    */
    static XPathValue decimalArithmetic(BinaryOperator op, const utils::Decimal& left,
        const utils::Decimal& right)
    {
        utils::Decimal result;
        bool defined = false;

        if (op == BinaryOperator::PLUS_OP)
            return XPathValue::fromDecimal(utils::addDecimals(left, right));
        if (op == BinaryOperator::MINUS_OP)
            return XPathValue::fromDecimal(utils::subtractDecimals(left, right));
        if (op == BinaryOperator::MULTIPLY_OP)
            return XPathValue::fromDecimal(utils::multiplyDecimals(left, right));
        if (op == BinaryOperator::DIVIDE_OP)
            defined = utils::divideDecimals(left, right, DECIMAL_DIVISION_SCALE, result);
        else
            defined = utils::moduloDecimals(left, right, result);
        if (!defined)
            failEvaluation(i18n::format(i18n::Message::XPATH_DIVISION_BY_ZERO));
        return XPathValue::fromDecimal(result);
    }

    /*
    ** + - * div mod: оба операнда атомизируются (не больше одного атома;
    ** пустой — результат NaN, как «пустая последовательность»). Точные
    ** операнды дают точный результат, иначе — double. Арифметики дат нет:
    ** для неё нужны длительности XPath 2.0 — это ошибка, а не тихий NaN.
    **
    ** + - * div mod: both operands are atomized (at most one atom;
    ** an empty one gives NaN, as "an empty sequence"). Exact
    ** operands give an exact result, otherwise — double. There is no date arithmetic:
    ** it needs XPath 2.0 durations — this is an error, not a silent NaN.
    */
    XPathValue typedArithmetic(BinaryOperator op, const XPathValue& left, const XPathValue& right,
        const EvalContext& context)
    {
        XPathValue a = XPathValue::fromBoolean(false);
        XPathValue b = XPathValue::fromBoolean(false);
        utils::Decimal x;
        utils::Decimal y;

        if (!singleAtom(left, context, a) || !singleAtom(right, context, b))
            return XPathValue::fromNumber(std::nan(""));
        if (isDateValue(a) || isDateValue(b))
            failEvaluation(i18n::format(i18n::Message::XPATH_DATE_ARITHMETIC));
        if (exactDecimal(a, x) && exactDecimal(b, y))
            return decimalArithmetic(op, x, y);
        return XPathValue::fromNumber(numberArithmetic(op, numericDouble(a), numericDouble(b)));
    }

    // Унарный минус: точный для decimal, иначе — у double
    // The unary minus: exact for a decimal, otherwise — for a double
    XPathValue typedNegate(const XPathValue& value, const EvalContext& context)
    {
        XPathValue atom = XPathValue::fromBoolean(false);
        utils::Decimal exact;

        if (!singleAtom(value, context, atom))
            return XPathValue::fromNumber(std::nan(""));
        if (isDateValue(atom))
            failEvaluation(i18n::format(i18n::Message::XPATH_DATE_ARITHMETIC));
        if (exactDecimal(atom, exact))
            return XPathValue::fromDecimal(utils::negateDecimal(exact));
        return XPathValue::fromNumber(-numericDouble(atom));
    }
}
