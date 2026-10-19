#include "xpath/xpath_value.hpp"

#include <utility>

namespace beresta::xpath
{
    // Точное десятичное число
    // An exact decimal number
    XPathValue XPathValue::fromDecimal(utils::Decimal number)
    {
        XPathValue value;

        value._type = ValueType::DECIMAL;
        value._decimal = std::move(number);
        return value;
    }

    // Дата или дата-время: запись хранится как строка, порядок — по ключу
    // A date or a date-time: the notation is kept as a string, the order goes by the key
    XPathValue XPathValue::fromDate(ValueType type, std::string lexical, long double key)
    {
        XPathValue value;

        value._type = type;
        value._string = std::move(lexical);
        value._dateKey = key;
        return value;
    }

    // Точное значение (у не-DECIMAL — нормализованный ноль)
    // The exact value (for non-DECIMAL — a normalized zero)
    const utils::Decimal& XPathValue::getDecimal() const noexcept
    {
        return _decimal;
    }

    // Ключ порядка даты (у остальных типов — ноль)
    // The order key of a date (zero for the other types)
    long double XPathValue::getDateKey() const noexcept
    {
        return _dateKey;
    }
}
