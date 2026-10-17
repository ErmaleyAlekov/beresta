#include "xpath/xpath_value.hpp"

#include <utility>

namespace beresta::xpath
{
    // Пустое значение создаётся только фабричными методами ниже
    XPathValue::XPathValue() noexcept
        : _type(ValueType::BOOLEAN), _nodes(), _boolean(false), _number(0.0), _string()
    {
    }

    // Множество узлов (порядок документа обеспечивает вызывающий)
    XPathValue XPathValue::fromNodeSet(std::vector<XPathItem> nodes)
    {
        XPathValue value;

        value._type = ValueType::NODE_SET;
        value._nodes = std::move(nodes);
        return value;
    }

    // Логическое значение
    XPathValue XPathValue::fromBoolean(bool flag)
    {
        XPathValue value;

        value._type = ValueType::BOOLEAN;
        value._boolean = flag;
        return value;
    }

    // Число
    XPathValue XPathValue::fromNumber(double number)
    {
        XPathValue value;

        value._type = ValueType::NUMBER;
        value._number = number;
        return value;
    }

    // Строка
    XPathValue XPathValue::fromString(std::string text)
    {
        XPathValue value;

        value._type = ValueType::STRING;
        value._string = std::move(text);
        return value;
    }
}
