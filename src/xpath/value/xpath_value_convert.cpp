#include "xpath/xpath_value.hpp"

#include <cmath>

namespace beresta::xpath
{
    // Тип значения
    ValueType XPathValue::getType() const noexcept
    {
        return _type;
    }

    // Узлы множества
    const std::vector<XPathItem>& XPathValue::getNodes() const noexcept
    {
        return _nodes;
    }

    // boolean(): непустое множество, ненулевое число (не NaN), непустая строка
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
