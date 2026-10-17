#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_functions.hpp"

namespace beresta::xpath::detail
{
    // Значение аргумента вычисляется в контексте вызова функции
    XPathValue argument(const FunctionCall& call, std::size_t index)
    {
        return call.arguments[index]->evaluate(call.context);
    }

    // Аргумент как строка; без аргумента — строковое значение контекстного узла
    std::string stringArgument(const FunctionCall& call, std::size_t index)
    {
        if (index >= call.arguments.size())
            return stringValue(call.context.item);
        return argument(call, index).toString();
    }

    // Аргумент как число; без аргумента — число из контекстного узла
    double numberArgument(const FunctionCall& call, std::size_t index)
    {
        if (index >= call.arguments.size())
            return stringToNumber(stringValue(call.context.item));
        return argument(call, index).toNumber();
    }

    /*
    ** Узел для local-name(), name() и т.п.: без аргумента — контекстный
    ** узел, с аргументом — первый узел множества (false, если множество пусто).
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
            failEvaluation("аргумент функции должен быть множеством узлов");
        if (value.getNodes().empty())
            return false;
        item = value.getNodes().front();
        return true;
    }
}
