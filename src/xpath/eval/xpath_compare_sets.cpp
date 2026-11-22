#include "xpath/internal/xpath_eval.hpp"
#include "utils/seeded_hash.hpp"

#include <cmath>
#include <limits>
#include <unordered_set>

namespace beresta::xpath::detail
{
    // Равенство требует общей строки; неравенство — хотя бы двух разных строк.
    // Equality requires a common string; inequality requires at least two distinct strings.
    static bool compareSetStrings(BinaryOperator op, const XPathValue& left, const XPathValue& right)
    {
        std::unordered_set<std::string, utils::SeededHash> strings;

        for (const XPathItem& item : left.getNodes())
            strings.insert(stringValue(item));
        if (op == BinaryOperator::NOT_EQUAL_OP && strings.size() > 1)
            return true;
        for (const XPathItem& item : right.getNodes())
        {
            bool found = strings.find(stringValue(item)) != strings.end();
            if (op == BinaryOperator::EQUAL_OP && found)
                return true;
            if (op == BinaryOperator::NOT_EQUAL_OP && !found)
                return true;
        }
        return false;
    }

    // Существование пары для < или <= определяется минимумом слева и максимумом справа.
    // The existence of a pair for < or <= is determined by the left minimum and right maximum.
    static bool compareSetNumbers(BinaryOperator op, const XPathValue& left, const XPathValue& right)
    {
        double minimum = std::numeric_limits<double>::quiet_NaN();
        double maximum = std::numeric_limits<double>::quiet_NaN();

        for (const XPathItem& item : left.getNodes())
        {
            double value = stringToNumber(stringValue(item));
            if (!std::isnan(value) && (std::isnan(minimum) || value < minimum))
                minimum = value;
        }
        for (const XPathItem& item : right.getNodes())
        {
            double value = stringToNumber(stringValue(item));
            if (!std::isnan(value) && (std::isnan(maximum) || value > maximum))
                maximum = value;
        }
        return compareNumbers(op, minimum, maximum);
    }

    // Пустое множество не имеет пар; > и >= сводятся к сравнению переставленных множеств.
    // An empty set has no pairs; > and >= reduce to comparing the swapped sets.
    bool compareNodeSets(BinaryOperator op, const XPathValue& left, const XPathValue& right)
    {
        if (left.getNodes().empty() || right.getNodes().empty())
            return false;
        if (op == BinaryOperator::EQUAL_OP || op == BinaryOperator::NOT_EQUAL_OP)
            return compareSetStrings(op, left, right);
        if (op == BinaryOperator::GREATER_OP)
            return compareSetNumbers(BinaryOperator::LESS_OP, right, left);
        if (op == BinaryOperator::GREATER_EQUAL_OP)
            return compareSetNumbers(BinaryOperator::LESS_EQUAL_OP, right, left);
        return compareSetNumbers(op, left, right);
    }
}
