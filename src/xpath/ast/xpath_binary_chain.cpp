#include "xpath/internal/xpath_ast.hpp"

#include <utility>
#include <vector>

namespace beresta::xpath::detail
{
    /*
    ** Левый операнд как бинарное выражение или nullptr. dynamic_cast выбран
    ** вместо нового виртуального метода у Expr: левый хребет нужен только
    ** BinaryExpr, и остальные классы дерева об этом знать не должны.
    **
    ** The left operand as a binary expression or nullptr. dynamic_cast is chosen
    ** over a new virtual method of Expr: only BinaryExpr needs the left spine,
    ** and the other tree classes should not know about it.
    */
    static const BinaryExpr* asBinary(const Expr* expression) noexcept
    {
        return dynamic_cast<const BinaryExpr*>(expression);
    }

    /*
    ** Вычисление без рекурсии по левому хребту. Узлы хребта собираются
    ** сверху вниз, затем самый левый лист вычисляется первым, и значение
    ** сворачивается снизу вверх: каждый узел применяет свою операцию к
    ** накопленному значению и своему правому операнду. Порядок вычисления
    ** и короткое замыкание or / and — те же, что у рекурсивного варианта.
    ** Одиночная операция (левый операнд — не бинарное выражение) обходится
    ** без вектора: это самый частый случай.
    **
    ** Evaluation without recursion over the left spine. The spine nodes are collected
    ** top-down, then the leftmost leaf is evaluated first, and the value is
    ** folded bottom-up: each node applies its operation to the
    ** accumulated value and its right operand. The evaluation order
    ** and the short-circuiting of or / and are the same as in the recursive variant.
    ** A single operation (the left operand is not a binary expression) goes
    ** without a vector: it is the most frequent case.
    */
    XPathValue BinaryExpr::evaluate(const EvalContext& context) const
    {
        if (asBinary(_left.get()) == nullptr)
            return combine(_left->evaluate(context), context);
        std::vector<const BinaryExpr*> spine;
        for (const BinaryExpr* node = this; node != nullptr; node = asBinary(node->_left.get()))
            spine.push_back(node);
        XPathValue value = spine.back()->_left->evaluate(context);
        for (std::size_t index = spine.size(); index > 0; --index)
            value = spine[index - 1]->combine(std::move(value), context);
        return value;
    }

    /*
    ** Разрушение без рекурсии по левому хребту: левый операнд каждого узла
    ** хребта забирается до того, как сам узел удаляется, поэтому деструктор
    ** удаляемого узла видит пустой левый операнд и вглубь не уходит. Правые
    ** операнды удаляются обычным путём: их глубина ограничена вложенностью
    ** выражения, а их собственные хребты разбирает тот же деструктор.
    **
    ** Destruction without recursion over the left spine: the left operand of each spine
    ** node is taken before the node itself is deleted, so the destructor
    ** of the deleted node sees an empty left operand and does not go deeper. The right
    ** operands are deleted the usual way: their depth is limited by the nesting
    ** of the expression, and their own spines are taken apart by the same destructor.
    */
    BinaryExpr::~BinaryExpr()
    {
        ExprPtr next = std::move(_left);

        while (next != nullptr)
        {
            BinaryExpr* binary = dynamic_cast<BinaryExpr*>(next.get());
            if (binary == nullptr)
                return;
            ExprPtr child = std::move(binary->_left);
            next = std::move(child);
        }
    }
}
