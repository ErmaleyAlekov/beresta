#include "xpath/internal/xpath_eval.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Условное выражение владеет условием и обеими ветвями
    // A conditional expression owns the condition and both branches
    IfExpr::IfExpr(ExprPtr condition, ExprPtr thenBranch, ExprPtr elseBranch) noexcept
        : _condition(std::move(condition)), _then(std::move(thenBranch)), _else(std::move(elseBranch))
    {
    }

    // Вычисляется только выбранная ветвь (ошибка в другой ветви не мешает)
    // Only the chosen branch is evaluated (an error in the other branch does not interfere)
    XPathValue IfExpr::evaluate(const EvalContext& context) const
    {
        if (_condition->evaluate(context).toBoolean())
            return _then->evaluate(context);
        return _else->evaluate(context);
    }

    // Квантор: вид, имя переменной, область значений и условие
    // A quantifier: the kind, the variable name, the domain and the condition
    QuantifiedExpr::QuantifiedExpr(bool every, std::string variable, ExprPtr domain, ExprPtr condition)
        : _every(every), _variable(std::move(variable)), _domain(std::move(domain)), _condition(std::move(condition))
    {
    }

    /*
    ** some — истинно, если условие верно хотя бы для одного члена области,
    ** every — если для всех (у пустой области: some — false, every — true).
    ** Член области — узел (переменная связывается с ним) или единственное
    ** атомарное значение. Перебор прекращается на первом решающем члене.
    **
    ** some is true if the condition holds for at least one member of the domain,
    ** every — if for all of them (for an empty domain: some — false, every — true).
    ** A member of the domain is a node (the variable is bound to it) or the only
    ** atomic value. The iteration stops at the first decisive member.
    */
    XPathValue QuantifiedExpr::evaluate(const EvalContext& context) const
    {
        XPathValue domain = _domain->evaluate(context);
        std::vector<XPathValue> members;

        if (domain.getType() != ValueType::NODE_SET)
            members.push_back(domain);
        for (const XPathItem& item : domain.getNodes())
            members.push_back(XPathValue::fromNodeSet(std::vector<XPathItem>{item}));
        for (const XPathValue& member : members)
        {
            LocalVariable binding{_variable, &member, context.locals};
            EvalContext inner{context.item, context.position, context.size, context.user, context.typed, &binding};
            bool satisfied = _condition->evaluate(inner).toBoolean();
            if (satisfied != _every)
                return XPathValue::fromBoolean(satisfied);
        }
        return XPathValue::fromBoolean(_every);
    }
}
