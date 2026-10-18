#include "xpath/internal/xpath_eval.hpp"
#include "i18n/i18n.hpp"
#include "xpath/internal/xpath_functions.hpp"

namespace beresta::xpath::detail
{
    // Значение аргумента вычисляется в контексте вызова функции
    // The argument value is evaluated in the context of the function call
    XPathValue argument(const FunctionCall& call, std::size_t index)
    {
        return call.arguments[index]->evaluate(call.context);
    }

    // Аргумент как строка; без аргумента — строковое значение контекстного узла
    // The argument as a string; without an argument — the string value of the context node
    std::string stringArgument(const FunctionCall& call, std::size_t index)
    {
        if (index >= call.arguments.size())
            return stringValue(call.context.item);
        return argument(call, index).toString();
    }

    // Аргумент как число; без аргумента — число из контекстного узла
    // The argument as a number; without an argument — the number from the context node
    double numberArgument(const FunctionCall& call, std::size_t index)
    {
        if (index >= call.arguments.size())
            return stringToNumber(stringValue(call.context.item));
        return argument(call, index).toNumber();
    }

    /*
    ** Узел для local-name(), name() и т.п.: без аргумента — контекстный
    ** узел, с аргументом — первый узел множества (false, если множество пусто).
    **
    ** The node for local-name(), name() and so on: without an argument — the context
    ** node, with an argument — the first node of the set (false if the set is empty).
    */
    bool nodeArgument(const FunctionCall& call, XPathItem& item)
    {
        if (call.arguments.empty())
        {
            item = call.context.item;
            return true;
        }
        XPathValue value = argument(call, 0);
        if (value.getType() != ValueType::NODE_SET)
            failEvaluation(i18n::format(i18n::Message::XPATH_ARGUMENT_NOT_NODESET));
        if (value.getNodes().empty())
            return false;
        item = value.getNodes().front();
        return true;
    }
}
