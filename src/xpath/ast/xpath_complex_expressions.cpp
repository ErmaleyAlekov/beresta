#include "xpath/internal/xpath_eval.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Значение переменной из пользовательского контекста; неизвестная — ошибка
    // The variable value from the user context; an unknown one is an error
    XPathValue VariableExpr::evaluate(const EvalContext& context) const
    {
        auto found = context.user.variables.find(_name);

        if (found == context.user.variables.end())
            failEvaluation(i18n::format(i18n::Message::XPATH_UNDEFINED_VARIABLE, {_name}));
        return found->second;
    }

    // Вызов функции хранит имя и аргументы
    // A function call keeps the name and the arguments
    FunctionCallExpr::FunctionCallExpr(std::string name, std::vector<ExprPtr> arguments)
        : _name(std::move(name)), _arguments(std::move(arguments))
    {
    }

    // Аргументы вычисляет сама функция: некоторым (boolean, count) нужен лишь их тип
    // The function evaluates the arguments itself: some (boolean, count) need only their type
    XPathValue FunctionCallExpr::evaluate(const EvalContext& context) const
    {
        return callFunction(_name, _arguments, context);
    }

    // Фильтр владеет первичным выражением и предикатами
    // A filter owns the primary expression and the predicates
    FilterExpr::FilterExpr(ExprPtr primary, std::vector<ExprPtr> predicates)
        : _primary(std::move(primary)), _predicates(std::move(predicates))
    {
    }

    /*
    ** (выражение)[предикат]: позиции в предикатах считаются в порядке
    ** документа (у фильтра нет «оси», а значит и обратного порядка).
    **
    ** (expression)[predicate]: positions in the predicates are counted in document
    ** order (a filter has no "axis", and so no reverse order).
    */
    XPathValue FilterExpr::evaluate(const EvalContext& context) const
    {
        XPathValue value = _primary->evaluate(context);

        if (value.getType() != ValueType::NODE_SET)
            failEvaluation(i18n::format(i18n::Message::XPATH_PREDICATE_NOT_NODESET));
        std::vector<XPathItem> items = value.getNodes();
        sortDocumentOrder(items);
        return XPathValue::fromNodeSet(applyPredicates(_predicates, std::move(items), context.user));
    }
}
