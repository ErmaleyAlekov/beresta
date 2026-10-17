#include "xpath/internal/xpath_eval.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Значение переменной из пользовательского контекста; неизвестная — ошибка
    XPathValue VariableExpr::evaluate(const EvalContext& context) const
    {
        auto found = context.user.variables.find(_name);

        if (found == context.user.variables.end())
            failEvaluation("переменная $" + _name + " не определена");
        return found->second;
    }

    // Вызов функции хранит имя и аргументы
    FunctionCallExpr::FunctionCallExpr(std::string name, std::vector<ExprPtr> arguments)
        : _name(std::move(name)), _arguments(std::move(arguments))
    {
    }

    // Аргументы вычисляет сама функция: некоторым (boolean, count) нужен лишь их тип
    XPathValue FunctionCallExpr::evaluate(const EvalContext& context) const
    {
        return callFunction(_name, _arguments, context);
    }

    // Фильтр владеет первичным выражением и предикатами
    FilterExpr::FilterExpr(ExprPtr primary, std::vector<ExprPtr> predicates)
        : _primary(std::move(primary)), _predicates(std::move(predicates))
    {
    }

    /*
    ** (выражение)[предикат]: позиции в предикатах считаются в порядке
    ** документа (у фильтра нет «оси», а значит и обратного порядка).
    */
    XPathValue FilterExpr::evaluate(const EvalContext& context) const
    {
        XPathValue value = _primary->evaluate(context);

        if (value.getType() != ValueType::NODE_SET)
            failEvaluation("предикат применим только к множеству узлов");
        std::vector<XPathItem> items = value.getNodes();
        sortDocumentOrder(items);
        return XPathValue::fromNodeSet(applyPredicates(_predicates, std::move(items), context.user));
    }
}
