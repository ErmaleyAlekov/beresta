#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_functions.hpp"

namespace beresta::xpath::detail
{
    // string(значение?): строковое представление аргумента или контекстного узла
    // string(value?): the string representation of the argument or the context node
    XPathValue functionString(const FunctionCall& call)
    {
        return XPathValue::fromString(stringArgument(call, 0));
    }

    // concat(a, b, ...): склейка всех аргументов
    // concat(a, b, ...): joins all arguments
    XPathValue functionConcat(const FunctionCall& call)
    {
        std::string result;

        for (std::size_t index = 0; index < call.arguments.size(); ++index)
            result += stringArgument(call, index);
        return XPathValue::fromString(result);
    }

    // starts-with(строка, префикс)
    // starts-with(string, prefix)
    XPathValue functionStartsWith(const FunctionCall& call)
    {
        std::string text = stringArgument(call, 0);
        std::string prefix = stringArgument(call, 1);

        return XPathValue::fromBoolean(text.compare(0, prefix.size(), prefix) == 0);
    }

    // contains(строка, подстрока)
    // contains(string, substring)
    XPathValue functionContains(const FunctionCall& call)
    {
        std::string text = stringArgument(call, 0);

        return XPathValue::fromBoolean(text.find(stringArgument(call, 1)) != std::string::npos);
    }

    // substring-before(строка, разделитель): часть до первого вхождения
    // substring-before(string, separator): the part before the first occurrence
    XPathValue functionSubstringBefore(const FunctionCall& call)
    {
        std::string text = stringArgument(call, 0);
        std::size_t found = text.find(stringArgument(call, 1));

        if (found == std::string::npos)
            return XPathValue::fromString("");
        return XPathValue::fromString(text.substr(0, found));
    }
}
