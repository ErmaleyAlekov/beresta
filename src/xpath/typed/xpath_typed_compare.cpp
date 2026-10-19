#include "xpath/internal/xpath_typed.hpp"

namespace beresta::xpath::detail
{
    // eq ne lt le gt ge -> = != < <= > >= (перечисления идут в одном порядке)
    // eq ne lt le gt ge -> = != < <= > >= (the enumerations go in the same order)
    static BinaryOperator generalOperator(BinaryOperator op) noexcept
    {
        int offset = static_cast<int>(op) - static_cast<int>(BinaryOperator::VALUE_EQUAL_OP);

        return static_cast<BinaryOperator>(static_cast<int>(BinaryOperator::EQUAL_OP) + offset);
    }

    /*
    ** Общее сравнение (= != < …): истинно, если условие выполняется хотя бы
    ** для одной пары атомов левого и правого операндов.
    **
    ** A general comparison (= != < …): true if the condition holds for at least
    ** one pair of atoms of the left and the right operands.
    */
    static bool compareGeneral(BinaryOperator op, const XPathValue& left, const XPathValue& right,
        const EvalContext& context)
    {
        std::vector<XPathValue> leftAtoms = atomizeValue(left, context);
        std::vector<XPathValue> rightAtoms = atomizeValue(right, context);

        for (const XPathValue& a : leftAtoms)
        {
            for (const XPathValue& b : rightAtoms)
            {
                if (compareTypedAtoms(op, a, b, context))
                    return true;
            }
        }
        return false;
    }

    // Сравнение значений (eq …): по одному атому с каждой стороны; пустой операнд — false
    // A value comparison (eq …): one atom on each side; an empty operand — false
    static bool compareSingles(BinaryOperator op, const XPathValue& left, const XPathValue& right,
        const EvalContext& context)
    {
        XPathValue a = XPathValue::fromBoolean(false);
        XPathValue b = XPathValue::fromBoolean(false);

        if (!singleAtom(left, context, a) || !singleAtom(right, context, b))
            return false;
        return compareTypedAtoms(generalOperator(op), a, b, context);
    }

    // Выбор по виду операции: сравнение значений, общее сравнение или арифметика
    // The choice by the kind of operation: a value comparison, a general comparison or arithmetic
    XPathValue typedBinary(BinaryOperator op, const XPathValue& left, const XPathValue& right,
        const EvalContext& context)
    {
        if (op >= BinaryOperator::VALUE_EQUAL_OP)
            return XPathValue::fromBoolean(compareSingles(op, left, right, context));
        if (op >= BinaryOperator::EQUAL_OP && op <= BinaryOperator::GREATER_EQUAL_OP)
            return XPathValue::fromBoolean(compareGeneral(op, left, right, context));
        return typedArithmetic(op, left, right, context);
    }
}
