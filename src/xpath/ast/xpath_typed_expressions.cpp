#include "xpath/internal/xpath_typed.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Конструктор типа владеет выражением-аргументом
    // A type constructor owns the argument expression
    ConstructorExpr::ConstructorExpr(AtomicType type, ExprPtr argument) noexcept
        : _type(type), _argument(std::move(argument))
    {
    }

    // Значение аргумента, приведённое к типу конструктора
    // The argument value cast to the constructor type
    XPathValue ConstructorExpr::evaluate(const EvalContext& context) const
    {
        return castAtomic(_type, _argument->evaluate(context), context);
    }

    // Корень утверждения оборачивает всё выражение
    // The assertion root wraps the whole expression
    AssertionRootExpr::AssertionRootExpr(ExprPtr inner) noexcept
        : _inner(std::move(inner))
    {
    }

    /*
    ** Валидатор передаёт свой источник типов; без него (прямой вызов
    ** XPathExpression::evaluate) подставляется нетипизированный — общий
    ** неизменяемый объект, безопасный для любых потоков.
    **
    ** The validator passes its own type source; without it (a direct call of
    ** XPathExpression::evaluate) the untyped one is substituted — a shared
    ** immutable object, safe for any threads.
    */
    XPathValue AssertionRootExpr::evaluate(const EvalContext& context) const
    {
        static const UntypedValueSource untyped{};

        if (context.typed != nullptr)
            return _inner->evaluate(context);
        EvalContext typed{context.item, context.position, context.size, context.user, &untyped, context.locals};
        return _inner->evaluate(typed);
    }
}
