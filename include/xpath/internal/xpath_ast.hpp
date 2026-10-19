#ifndef XPATH_AST_HPP
#define XPATH_AST_HPP

#include "xpath/xpath.hpp"
#include "xpath/xpath_value.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::xpath::detail
{
    class TypedValueSource;

    /*
    ** Локальная переменная: $x кванторов some / every и $value фасета
    ** xs:assertion. Привязки образуют односвязный список на стеке
    ** вычисления: внутренняя привязка закрывает внешнюю с тем же именем.
    **
    ** A local variable: $x of the some / every quantifiers and $value of the
    ** xs:assertion facet. The bindings form a singly linked list on the evaluation
    ** stack: an inner binding hides an outer one with the same name.
    */
    struct LocalVariable
    {
        std::string_view name;
        const XPathValue* value;
        const LocalVariable* next;
    };

    /*
    ** Контекст вычисления выражения: текущий узел, его позиция и размер
    ** контекстного множества (для position() и last()), пользовательский
    ** контекст (префиксы и переменные), локальные переменные и источник
    ** типизированных значений. typed != nullptr — выражение вычисляется
    ** по правилам утверждений XSD 1.1 (подмножество XPath 2.0): узлы
    ** атомизируются в значения своих типов, числа точные.
    **
    ** The expression evaluation context: the current node, its position and the size
    ** of the context set (for position() and last()), the user
    ** context (prefixes and variables), the local variables and the source
    ** of typed values. typed != nullptr — the expression is evaluated
    ** by the rules of XSD 1.1 assertions (a subset of XPath 2.0): nodes
    ** are atomized into the values of their types, numbers are exact.
    */
    struct EvalContext
    {
        XPathItem item;
        std::size_t position;
        std::size_t size;
        const XPathContext& user;
        const TypedValueSource* typed = nullptr;
        const LocalVariable* locals = nullptr;
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

    /*
    ** Бинарные операции (суффикс _OP — чтобы не столкнуться с макросами
    ** платформ). VALUE_* — сравнения значений XPath 2.0 (eq ne lt le gt ge):
    ** они есть только в утверждениях XSD и идут в том же порядке, что и
    ** общие сравнения = != < <= > >=.
    **
    ** Binary operations (the _OP suffix avoids clashes with platform
    ** macros). VALUE_* are the XPath 2.0 value comparisons (eq ne lt le gt ge):
    ** they exist only in XSD assertions and go in the same order as the
    ** general comparisons = != < <= > >=.
    */
    enum class BinaryOperator
    {
        OR_OP, AND_OP, EQUAL_OP, NOT_EQUAL_OP, LESS_OP, LESS_EQUAL_OP, GREATER_OP,
        GREATER_EQUAL_OP, PLUS_OP, MINUS_OP, MULTIPLY_OP, DIVIDE_OP, MODULO_OP, UNION_OP,
        VALUE_EQUAL_OP, VALUE_NOT_EQUAL_OP, VALUE_LESS_OP, VALUE_LESS_EQUAL_OP, VALUE_GREATER_OP,
        VALUE_GREATER_EQUAL_OP
    };

    // Тип конструктора xs:тип(значение) в утверждениях XSD
    // The type of an xs:type(value) constructor in XSD assertions
    enum class AtomicType
    {
        STRING, BOOLEAN, DECIMAL, INTEGER, DOUBLE, DATE, DATE_TIME
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

    // ---- Конструкции XPath 2.0 утверждений XSD (xpath_conditional.cpp, xpath_typed_expressions.cpp) ----
    // ---- XPath 2.0 constructs of XSD assertions (xpath_conditional.cpp, xpath_typed_expressions.cpp) ----

    // if (условие) then выражение else выражение
    // if (condition) then expression else expression
    class IfExpr : public Expr
    {
    public:
        IfExpr(ExprPtr condition, ExprPtr thenBranch, ExprPtr elseBranch) noexcept;

        XPathValue evaluate(const EvalContext& context) const override;

    private:
        ExprPtr _condition;
        ExprPtr _then;
        ExprPtr _else;
    };

    /*
    ** some / every $имя in последовательность satisfies условие. Несколько
    ** привязок ($a in X, $b in Y) парсер разворачивает во вложенные кванторы.
    **
    ** some / every $name in sequence satisfies condition. Several
    ** bindings ($a in X, $b in Y) are unfolded by the parser into nested quantifiers.
    */
    class QuantifiedExpr : public Expr
    {
    public:
        QuantifiedExpr(bool every, std::string variable, ExprPtr domain, ExprPtr condition);

        XPathValue evaluate(const EvalContext& context) const override;

    private:
        bool _every;
        std::string _variable;
        ExprPtr _domain;
        ExprPtr _condition;
    };

    // Конструктор xs:decimal(…), xs:date(…) и т.п.: приведение одного значения к типу
    // The constructor xs:decimal(…), xs:date(…) and so on: casting one value to the type
    class ConstructorExpr : public Expr
    {
    public:
        ConstructorExpr(AtomicType type, ExprPtr argument) noexcept;

        XPathValue evaluate(const EvalContext& context) const override;

    private:
        AtomicType _type;
        ExprPtr _argument;
    };

    /*
    ** Корень выражения-утверждения: гарантирует типизированный режим. Если
    ** выражение вычисляют через общий XPathExpression::evaluate (без схемы),
    ** узлы атомизируются как нетипизированные строки (паттерн Null Object).
    **
    ** The root of an assertion expression: guarantees the typed mode. If
    ** the expression is evaluated through the common XPathExpression::evaluate (without a schema),
    ** nodes are atomized as untyped strings (the Null Object pattern).
    */
    class AssertionRootExpr : public Expr
    {
    public:
        explicit AssertionRootExpr(ExprPtr inner) noexcept;

        XPathValue evaluate(const EvalContext& context) const override;

    private:
        ExprPtr _inner;
    };
}

#endif
