#include "xpath/xpath.hpp"

namespace beresta::xpath
{
    // Элементы по выражению (изменяемое дерево)
    // Elements by an expression (a mutable tree)
    std::vector<Element*> selectElements(Node& context, std::string_view expression)
    {
        return XPathExpression(expression).selectElements(context);
    }

    // Элементы по выражению (константное дерево)
    // Elements by an expression (a const tree)
    std::vector<const Element*> selectElements(const Node& context, std::string_view expression)
    {
        return XPathExpression(expression).selectElements(context);
    }

    // Строковое значение выражения (string(...) от результата)
    // The string value of an expression (string(...) of the result)
    std::string evaluateString(const Node& context, std::string_view expression)
    {
        return XPathExpression(expression).evaluate(context).toString();
    }
}
