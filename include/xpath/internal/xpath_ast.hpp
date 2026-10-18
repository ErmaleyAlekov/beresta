#ifndef XPATH_AST_HPP
#define XPATH_AST_HPP

#include "xpath/xpath.hpp"
#include "xpath/xpath_value.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace beresta::xpath::detail
{
    /*
    ** Контекст вычисления выражения: текущий узел, его позиция и размер
    ** контекстного множества (для position() и last()) и пользовательский
    ** контекст (префиксы и переменные).
    **
    ** The expression evaluation context: the current node, its position and the size
    ** of the context set (for position() and last()) and the user
    ** context (prefixes and variables).
    */
    struct EvalContext
    {
        XPathItem item;
        std::size_t position;
        std::size_t size;
        const XPathContext& user;
    };

    /*
    ** Expr — узел дерева разбора (паттерн Interpreter, GoF): каждый класс
    ** выражения сам умеет вычислять себя. Добавление нового вида выражения
    ** не затрагивает остальные (принцип открытости/закрытости).
    **
    ** Expr — a parse tree node (the Interpreter pattern, GoF): each expression
    ** class knows how to evaluate itself. Adding a new kind of expression
    ** does not affect the others (the open/closed principle).
    */
    class Expr
    {
    public:
        virtual ~Expr();

        virtual XPathValue evaluate(const EvalContext& context) const = 0;
    };

    using ExprPtr = std::unique_ptr<Expr>;

    // Бинарные операции (суффикс _OP — чтобы не столкнуться с макросами платформ)
    // Binary operations (the _OP suffix avoids clashes with platform macros)
    enum class BinaryOperator
    {
        OR_OP, AND_OP, EQUAL_OP, NOT_EQUAL_OP, LESS_OP, LESS_EQUAL_OP, GREATER_OP,
        GREATER_EQUAL_OP, PLUS_OP, MINUS_OP, MULTIPLY_OP, DIVIDE_OP, MODULO_OP, UNION_OP
    };

    // Бинарное выражение: логика, сравнения, арифметика, объединение множеств
    // A binary expression: logic, comparisons, arithmetic, set union
    class BinaryExpr : public Expr
    {
    public:
        BinaryExpr(BinaryOperator op, ExprPtr left, ExprPtr right) noexcept;

        XPathValue evaluate(const EvalContext& context) const override;

    private:
        BinaryOperator _operator;
        ExprPtr _left;
        ExprPtr _right;
    };

    // Унарный минус
    // Unary minus
    class NegateExpr : public Expr
    {
    public:
        explicit NegateExpr(ExprPtr operand) noexcept;

        XPathValue evaluate(const EvalContext& context) const override;

    private:
        ExprPtr _operand;
    };

    // Строковый литерал или число — константа
    // A string literal or a number — a constant
    class ConstantExpr : public Expr
    {
    public:
        explicit ConstantExpr(XPathValue value);

        XPathValue evaluate(const EvalContext& context) const override;

    private:
        XPathValue _value;
    };

    // Ссылка на переменную $имя
    // A reference to a variable $name
    class VariableExpr : public Expr
    {
    public:
        explicit VariableExpr(std::string name);

        XPathValue evaluate(const EvalContext& context) const override;

    private:
        std::string _name;
    };

    // Вызов функции из базовой библиотеки XPath
    // A call of a function from the XPath core library
    class FunctionCallExpr : public Expr
    {
    public:
        FunctionCallExpr(std::string name, std::vector<ExprPtr> arguments);

        XPathValue evaluate(const EvalContext& context) const override;

    private:
        std::string _name;
        std::vector<ExprPtr> _arguments;
    };

    // Первичное выражение с предикатами: (выражение)[предикат]
    // A primary expression with predicates: (expression)[predicate]
    class FilterExpr : public Expr
    {
    public:
        FilterExpr(ExprPtr primary, std::vector<ExprPtr> predicates);

        XPathValue evaluate(const EvalContext& context) const override;

    private:
        ExprPtr _primary;
        std::vector<ExprPtr> _predicates;
    };

    // Все 13 осей XPath 1.0, включая namespace
    // All 13 XPath 1.0 axes, including namespace
    enum class Axis
    {
        ANCESTOR, ANCESTOR_OR_SELF, ATTRIBUTE_AXIS, CHILD, DESCENDANT, DESCENDANT_OR_SELF,
        FOLLOWING, FOLLOWING_SIBLING, PARENT, PRECEDING, PRECEDING_SIBLING, SELF,
        NAMESPACE_AXIS
    };

    // Вид проверки узла в шаге пути
    // The kind of node test in a path step
    enum class TestKind
    {
        QUALIFIED_NAME,     // имя или prefix:имя / a name or prefix:name
        ANY_NAME,           // *
        NAMESPACE_ANY,      // prefix:*
        ANY_NODE,           // node()
        TEXT_NODE,          // text()
        COMMENT_NODE,       // comment()
        PI_NODE             // processing-instruction('цель'?) / processing-instruction('target'?)
    };

    // Шаг пути: ось::проверка[предикаты]
    // A path step: axis::test[predicates]
    struct Step
    {
        Axis axis = Axis::CHILD;
        TestKind test = TestKind::ANY_NODE;
        std::string prefix;
        std::string localName;
        std::string target;
        std::vector<ExprPtr> predicates;
    };

    // Путь: необязательное начальное выражение или корень, затем шаги
    // A path: an optional initial expression or the root, then the steps
    class PathExpr : public Expr
    {
    public:
        PathExpr(ExprPtr start, bool absolute, std::vector<Step> steps);

        XPathValue evaluate(const EvalContext& context) const override;

    private:
        ExprPtr _start;
        bool _absolute;
        std::vector<Step> _steps;
    };
}

#endif
