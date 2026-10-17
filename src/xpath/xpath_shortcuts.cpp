#include "xpath/xpath.hpp"

namespace beresta::xpath
{
    // Элементы по выражению (изменяемое дерево)
    std::vector<Element*> selectElements(Node& context, std::string_view expression)
    {
        return XPathExpression(expression).selectElements(context);
    }

    // Элементы по выражению (константное дерево)
    std::vector<const Element*> selectElements(const Node& context, std::string_view expression)
    {
        return XPathExpression(expression).selectElements(context);
    }

    // Строковое значение выражения (string(...) от результата)
    std::string evaluateString(const Node& context, std::string_view expression)
    {
        return XPathExpression(expression).evaluate(context).toString();
    }
}
