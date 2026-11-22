#include "xpath/xpath_value.hpp"

#include <utility>

namespace beresta::xpath
{
    // Отличаем xs:untypedAtomic от строкового литерала без изменения XPath 1.0.
    // Distinguishes xs:untypedAtomic from a string literal without changing XPath 1.0.
    XPathValue XPathValue::fromUntypedString(std::string text)
    {
        XPathValue value = fromString(std::move(text));

        value._untyped = true;
        return value;
    }

    // Признак сохраняется при копировании атомизированного значения.
    // The marker is preserved when an atomized value is copied.
    bool XPathValue::isUntypedAtomic() const noexcept
    {
        return _untyped;
    }
}
