#include "xpath/xpath_value.hpp"

#include <utility>

namespace beresta::xpath
{
    // Пустое значение создаётся только фабричными методами ниже
    // An empty value is created only by the factory methods below
    XPathValue::XPathValue() noexcept
        : _type(ValueType::BOOLEAN), _nodes(), _boolean(false), _number(0.0), _string()
    {
    }

    // Множество узлов (порядок документа обеспечивает вызывающий)
    // A node set (the caller ensures document order)
    XPathValue XPathValue::fromNodeSet(std::vector<XPathItem> nodes)
    {
        XPathValue value;

        value._type = ValueType::NODE_SET;
        value._nodes = std::move(nodes);
        return value;
    }

    // Логическое значение
    // A boolean
    XPathValue XPathValue::fromBoolean(bool flag)
    {
        XPathValue value;

        value._type = ValueType::BOOLEAN;
        value._boolean = flag;
        return value;
    }

    // Число
    // A number
    XPathValue XPathValue::fromNumber(double number)
    {
        XPathValue value;

        value._type = ValueType::NUMBER;
        value._number = number;
        return value;
    }

    // Строка
    // A string
    XPathValue XPathValue::fromString(std::string text)
    {
        XPathValue value;

        value._type = ValueType::STRING;
        value._string = std::move(text);
        return value;
    }
}
