#include "xpath/internal/xpath_typed.hpp"
#include "xpath/internal/xpath_eval.hpp"

namespace beresta::xpath::detail
{
    // Подходит ли порядок под оператор (order — любое число со знаком результата сравнения)
    // Whether the order fits the operator (order is any number with the sign of the comparison result)
    bool orderMatches(BinaryOperator op, int order) noexcept
    {
        switch (op)
        {
            case BinaryOperator::EQUAL_OP:
                return order == 0;
            case BinaryOperator::NOT_EQUAL_OP:
                return order != 0;
            case BinaryOperator::LESS_OP:
                return order < 0;
            case BinaryOperator::LESS_EQUAL_OP:
                return order <= 0;
            case BinaryOperator::GREATER_OP:
                return order > 0;
            default:
                return order >= 0;
        }
    }

    /*
    ** С датой сравнивается дата того же типа: строка приводится к типу
    ** второго операнда (xs:date / xs:dateTime), иное — ошибка приведения.
    ** Порядок — по ключу-моменту в UTC.
    **
    ** A date is compared with a date of the same type: a string is cast to the type
    ** of the other operand (xs:date / xs:dateTime), anything else is a cast error.
    ** The order goes by the moment key in UTC.
    */
    static bool compareDateAtoms(BinaryOperator op, const XPathValue& left, const XPathValue& right,
        const EvalContext& context)
    {
        ValueType kind = right.getType();
        AtomicType type = AtomicType::DATE_TIME;
        int order = 0;

        if (isDateValue(left))
            kind = left.getType();
        if (kind == ValueType::DATE)
            type = AtomicType::DATE;
        long double a = castAtomic(type, left, context).getDateKey();
        long double b = castAtomic(type, right, context).getDateKey();
        if (a < b)
            order = -1;
        else if (a > b)
            order = 1;
        return orderMatches(op, order);
    }

    // С логическим значением сравнивается логическое: другой операнд приводится к xs:boolean
    // A boolean is compared with a boolean: the other operand is cast to xs:boolean
    static bool compareBooleanAtoms(BinaryOperator op, const XPathValue& left, const XPathValue& right,
        const EvalContext& context)
    {
        int a = static_cast<int>(castAtomic(AtomicType::BOOLEAN, left, context).toBoolean());
        int b = static_cast<int>(castAtomic(AtomicType::BOOLEAN, right, context).toBoolean());

        return orderMatches(op, a - b);
    }

    /*
    ** Числа: если оба операнда точны (decimal, запись decimal, целое
    ** double) — точное сравнение без double (0.1 + 0.2 = 0.3 истинно);
    ** иначе — как double по IEEE 754 (xs:double, NaN).
    **
    ** Numbers: if both operands are exact (a decimal, a decimal notation, an integer
    ** double) — an exact comparison without double (0.1 + 0.2 = 0.3 is true);
    ** otherwise — as double by IEEE 754 (xs:double, NaN).
    */
    static bool compareNumericAtoms(BinaryOperator op, const XPathValue& left, const XPathValue& right)
    {
        utils::Decimal a;
        utils::Decimal b;

        if (exactDecimal(left, a) && exactDecimal(right, b))
            return orderMatches(op, utils::compareDecimals(a, b));
        return compareNumbers(op, numericDouble(left), numericDouble(right));
    }

    /*
    ** Сравнение атомов XPath 2.0: дата, затем логическое значение, затем
    ** число (нетипизированная строка приводится к числу), иначе строки по
    ** кодовым точкам — и для < >, а не через number(), как в XPath 1.0.
    **
    ** Comparing XPath 2.0 atoms: a date, then a boolean, then
    ** a number (an untyped string is cast to a number), otherwise strings by
    ** code points — also for < >, not through number() as in XPath 1.0.
    */
    bool compareTypedAtoms(BinaryOperator op, const XPathValue& left, const XPathValue& right,
        const EvalContext& context)
    {
        bool numeric = left.getType() == ValueType::NUMBER || left.getType() == ValueType::DECIMAL
            || right.getType() == ValueType::NUMBER || right.getType() == ValueType::DECIMAL;

        if (isDateValue(left) || isDateValue(right))
            return compareDateAtoms(op, left, right, context);
        if (left.getType() == ValueType::BOOLEAN || right.getType() == ValueType::BOOLEAN)
            return compareBooleanAtoms(op, left, right, context);
        if (numeric)
            return compareNumericAtoms(op, left, right);
        return orderMatches(op, left.toString().compare(right.toString()));
    }
}
