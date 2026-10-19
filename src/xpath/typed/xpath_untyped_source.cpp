#include "xpath/internal/xpath_typed.hpp"

namespace beresta::xpath::detail
{
    // Виртуальный деструктор стратегии типов
    // The virtual destructor of the type strategy
    TypedValueSource::~TypedValueSource()
    {
    }

    // Без схемы у узла есть только строковое значение (xs:untypedAtomic)
    // Without a schema a node has only its string value (xs:untypedAtomic)
    XPathValue UntypedValueSource::atomize(const XPathItem& item) const
    {
        return XPathValue::fromString(stringValue(item));
    }

    // Без типов данных XSD даты не разбираются: конструктор xs:date даст ошибку приведения
    // Without the XSD datatypes dates are not parsed: the xs:date constructor gives a cast error
    bool UntypedValueSource::castDate(ValueType type, std::string_view text, XPathValue& result) const
    {
        (void)type;
        (void)text;
        (void)result;
        return false;
    }
}
