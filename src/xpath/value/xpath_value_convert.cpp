#include "xpath/xpath_value.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

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

    /*
    ** boolean(): непустое множество, ненулевое число (не NaN), непустая
    ** строка. У даты логического значения нет (ошибка FORG0006 XPath 2.0).
    **
    ** boolean(): a non-empty set, a non-zero number (not NaN), a non-empty
    ** string. A date has no boolean value (the XPath 2.0 error FORG0006).
    */
    bool XPathValue::toBoolean() const
    {
        if (_type == ValueType::NODE_SET)
            return !_nodes.empty();
        if (_type == ValueType::NUMBER)
            return _number != 0.0 && !std::isnan(_number);
        if (_type == ValueType::STRING)
            return !_string.empty();
        if (_type == ValueType::DECIMAL)
            return !_decimal.digits.empty();
        if (_type == ValueType::DATE || _type == ValueType::DATE_TIME)
            throw DomException(DomErrorCode::XPATH_ERROR, "XPath: "
                + i18n::format(i18n::Message::XPATH_DATE_BOOLEAN, {_string}));
        return _boolean;
    }

    // number(): для множества — число из строкового значения первого узла; у даты — NaN
    // number(): for a set — the number from the string value of the first node; for a date — NaN
    double XPathValue::toNumber() const
    {
        if (_type == ValueType::NUMBER)
            return _number;
        if (_type == ValueType::DECIMAL)
            return utils::decimalToDouble(_decimal);
        if (_type == ValueType::DATE || _type == ValueType::DATE_TIME)
            return std::nan("");
        if (_type == ValueType::BOOLEAN)
        {
            if (_boolean)
                return 1.0;
            return 0.0;
        }
        return stringToNumber(toString());
    }

    // string(): для множества — строковое значение первого узла; decimal — каноническая запись
    // string(): for a set — the string value of the first node; a decimal — the canonical notation
    std::string XPathValue::toString() const
    {
        if (_type == ValueType::STRING || _type == ValueType::DATE || _type == ValueType::DATE_TIME)
            return _string;
        if (_type == ValueType::DECIMAL)
            return utils::formatDecimal(_decimal);
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
