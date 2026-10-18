#include "xpath/xpath_value.hpp"

#include <cmath>

namespace beresta::xpath
{
    // Тип значения
    // The value type
    ValueType XPathValue::getType() const noexcept
    {
        return _type;
    }

    // Узлы множества
    // The nodes of the set
    const std::vector<XPathItem>& XPathValue::getNodes() const noexcept
    {
        return _nodes;
    }

    // boolean(): непустое множество, ненулевое число (не NaN), непустая строка
    // boolean(): a non-empty set, a non-zero number (not NaN), a non-empty string
    bool XPathValue::toBoolean() const
    {
        if (_type == ValueType::NODE_SET)
            return !_nodes.empty();
        if (_type == ValueType::NUMBER)
            return _number != 0.0 && !std::isnan(_number);
        if (_type == ValueType::STRING)
            return !_string.empty();
        return _boolean;
    }

    // number(): для множества — число из строкового значения первого узла
    // number(): for a set — the number from the string value of the first node
    double XPathValue::toNumber() const
    {
        if (_type == ValueType::NUMBER)
            return _number;
        if (_type == ValueType::BOOLEAN)
        {
            if (_boolean)
                return 1.0;
            return 0.0;
        }
        return stringToNumber(toString());
    }

    // string(): для множества — строковое значение первого узла в порядке документа
    // string(): for a set — the string value of the first node in document order
    std::string XPathValue::toString() const
    {
        if (_type == ValueType::STRING)
            return _string;
        if (_type == ValueType::NUMBER)
            return numberToString(_number);
        if (_type == ValueType::BOOLEAN)
        {
            if (_boolean)
                return "true";
            return "false";
        }
        if (_nodes.empty())
            return std::string();
        return stringValue(_nodes.front());
    }
}
