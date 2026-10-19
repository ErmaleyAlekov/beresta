#include "xpath/internal/xpath_eval.hpp"

namespace beresta::xpath::detail
{
    // Сравнение чисел (NaN ни с чем не равен — так ведёт себя IEEE 754)
    // Comparing numbers (NaN equals nothing — this is how IEEE 754 behaves)
    bool compareNumbers(BinaryOperator op, double left, double right) noexcept
    {
        switch (op)
        {
            case BinaryOperator::EQUAL_OP:
                return left == right;
            case BinaryOperator::NOT_EQUAL_OP:
                return left != right;
            case BinaryOperator::LESS_OP:
                return left < right;
            case BinaryOperator::LESS_EQUAL_OP:
                return left <= right;
            case BinaryOperator::GREATER_OP:
                return left > right;
            default:
                return left >= right;
        }
    }

    /*
    ** Сравнение двух не-множеств (раздел 3.4): для = и != — как логические,
    ** если одно из них логическое; иначе как числа, если одно из них число;
    ** иначе как строки. Для < <= > >= — всегда как числа.
    **
    ** Comparing two non-sets (section 3.4): for = and != — as booleans
    ** if one of them is a boolean; otherwise as numbers if one of them is a number;
    ** otherwise as strings. For < <= > >= — always as numbers.
    */
    static bool compareAtoms(BinaryOperator op, const XPathValue& left, const XPathValue& right)
    {
        bool equality = op == BinaryOperator::EQUAL_OP || op == BinaryOperator::NOT_EQUAL_OP;

        if (!equality)
            return compareNumbers(op, left.toNumber(), right.toNumber());
        bool equal = false;
        if (left.getType() == ValueType::BOOLEAN || right.getType() == ValueType::BOOLEAN)
            equal = left.toBoolean() == right.toBoolean();
        else if (left.getType() == ValueType::NUMBER || right.getType() == ValueType::NUMBER)
            return compareNumbers(op, left.toNumber(), right.toNumber());
        else
            equal = left.toString() == right.toString();
        if (op == BinaryOperator::EQUAL_OP)
            return equal;
        return !equal;
    }

    // Атом из строкового значения узла: число, если сравниваем с числом, иначе строка
    // An atom from the string value of a node: a number if compared with a number, otherwise a string
    static XPathValue atomFor(const XPathItem& item, const XPathValue& other)
    {
        if (other.getType() == ValueType::NUMBER)
            return XPathValue::fromNumber(stringToNumber(stringValue(item)));
        return XPathValue::fromString(stringValue(item));
    }

    /*
    ** Множество против не-множества: с логическим значением сравнивается
    ** boolean(множества), иначе условие истинно, если оно истинно хотя бы
    ** для одного узла. setOnLeft сохраняет исходный порядок операндов.
    **
    ** A set against a non-set: with a boolean the boolean(set) is compared,
    ** otherwise the condition is true if it is true for at least
    ** one node. setOnLeft keeps the original order of the operands.
    */
    static bool compareSetWith(BinaryOperator op, const XPathValue& set, const XPathValue& other,
        bool setOnLeft)
    {
        if (other.getType() == ValueType::BOOLEAN)
        {
            XPathValue flag = XPathValue::fromBoolean(set.toBoolean());
            if (setOnLeft)
                return compareAtoms(op, flag, other);
            return compareAtoms(op, other, flag);
        }
        for (const XPathItem& item : set.getNodes())
        {
            XPathValue atom = atomFor(item, other);
            if (setOnLeft && compareAtoms(op, atom, other))
                return true;
            if (!setOnLeft && compareAtoms(op, other, atom))
                return true;
        }
        return false;
    }

    // Два множества: истинно, если условие выполняется хотя бы для одной пары узлов
    // Two sets: true if the condition holds for at least one pair of nodes
    bool compareValues(BinaryOperator op, const XPathValue& left, const XPathValue& right)
    {
        bool leftSet = left.getType() == ValueType::NODE_SET;
        bool rightSet = right.getType() == ValueType::NODE_SET;

        if (leftSet && rightSet)
        {
            for (const XPathItem& item : left.getNodes())
            {
                if (compareSetWith(op, right, XPathValue::fromString(stringValue(item)), false))
                    return true;
            }
            return false;
        }
        if (leftSet)
            return compareSetWith(op, left, right, true);
        if (rightSet)
            return compareSetWith(op, right, left, false);
        return compareAtoms(op, left, right);
    }
}
