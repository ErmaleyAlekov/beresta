#include "xpath/internal/xpath_functions.hpp"

namespace beresta::xpath::detail
{
    // boolean(значение): приведение к логическому типу
    // boolean(value): conversion to the boolean type
    XPathValue functionBoolean(const FunctionCall& call)
    {
        return XPathValue::fromBoolean(argument(call, 0).toBoolean());
    }

    // not(значение): логическое отрицание
    // not(value): logical negation
    XPathValue functionNot(const FunctionCall& call)
    {
        return XPathValue::fromBoolean(!argument(call, 0).toBoolean());
    }

    // true()
    XPathValue functionTrue(const FunctionCall& call)
    {
        (void)call;
        return XPathValue::fromBoolean(true);
    }

    // false()
    XPathValue functionFalse(const FunctionCall& call)
    {
        (void)call;
        return XPathValue::fromBoolean(false);
    }
}
