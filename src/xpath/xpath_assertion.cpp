#include "xpath/internal/xpath_assertion.hpp"
#include "xpath/internal/xpath_parser.hpp"

#include <utility>

namespace beresta::xpath
{
    // Готовое дерево разбора и его исходный текст (только для ExpressionAccess)
    // A ready parse tree and its source text (only for ExpressionAccess)
    XPathExpression::XPathExpression(std::string source, std::shared_ptr<const detail::Expr> root) noexcept
        : _source(std::move(source)), _root(std::move(root))
    {
    }
}

namespace beresta::xpath::detail
{
    /*
    ** Лексер и парсер в диалекте утверждений; корень дерева — обёртка,
    ** гарантирующая типизированный режим при любом способе вычисления.
    **
    ** The lexer and the parser in the assertion dialect; the tree root is a wrapper
    ** guaranteeing the typed mode with any way of evaluation.
    */
    XPathExpression ExpressionAccess::compileAssertion(std::string_view text, const XPathContext& namespaces)
    {
        XPathLexer lexer(text, XPathDialect::XSD_ASSERTION);
        XPathParser parser(lexer.tokenize(), XPathDialect::XSD_ASSERTION, &namespaces);
        ExprPtr root = std::make_unique<AssertionRootExpr>(parser.parse());

        return XPathExpression(std::string(text), std::shared_ptr<const Expr>(std::move(root)));
    }

    // Контекст — узел в позиции 1 из 1; $value — локальная привязка, а не копия карты переменных
    // The context is the node at position 1 of 1; $value is a local binding rather than a copy of the variable map
    bool ExpressionAccess::evaluateAssertion(const XPathExpression& test, const Node& context,
        const XPathContext& namespaces, const TypedValueSource& types, const XPathValue* value)
    {
        LocalVariable binding{"value", value, nullptr};
        const LocalVariable* locals = nullptr;

        if (value != nullptr)
            locals = &binding;
        EvalContext evalContext{XPathItem{&context, nullptr, nullptr}, 1, 1, namespaces, &types, locals};
        return test._root->evaluate(evalContext).toBoolean();
    }
}
