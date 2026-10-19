#include "xpath/internal/xpath_eval.hpp"

#include <algorithm>
#include <utility>

namespace beresta::xpath::detail
{
    // Обратные оси выдают узлы от ближнего к дальнему, то есть против порядка документа
    // Reverse axes give nodes from the nearest to the farthest, that is against document order
    static bool isReverseAxis(Axis axis) noexcept
    {
        return axis == Axis::ANCESTOR || axis == Axis::ANCESTOR_OR_SELF
            || axis == Axis::PRECEDING || axis == Axis::PRECEDING_SIBLING;
    }

    /*
    ** Порядок итога шага. У ОДНОГО входного узла прямая ось уже выдаёт узлы
    ** в порядке документа и без повторов, а обратную достаточно развернуть, —
    ** сортировка не нужна (ось namespace — исключение: порядок её узлов
    ** задаёт ключ сортировки). Иначе — общая сортировка с проверкой «уже по
    ** порядку». Так //item не сортирует сотни тысяч узлов шага descendant-or-self.
    **
    ** The order of a step result. For ONE input node a forward axis already gives
    ** nodes in document order without duplicates, and a reverse one only needs to be
    ** reversed — no sort is needed (the namespace axis is an exception: the order of
    ** its nodes is defined by the sort key). Otherwise — the general sort with an
    ** "already in order" check. So //item does not sort hundreds of thousands of
    ** nodes of the descendant-or-self step.
    */
    static void orderStepResult(Axis axis, std::size_t inputCount, std::vector<XPathItem>& result)
    {
        if (inputCount == 1 && axis != Axis::NAMESPACE_AXIS)
        {
            if (isReverseAxis(axis))
                std::reverse(result.begin(), result.end());
            return;
        }
        sortDocumentOrder(result);
    }

    /*
    ** Шаг пути. Для каждого входного узла: узлы оси -> проверка узла ->
    ** предикаты (позиции — в порядке оси, для обратных осей — от ближнего).
    ** Буфер оси один на весь шаг; без предикатов подходящие узлы сразу идут
    ** в итог — без промежуточного списка на каждый входной узел.
    **
    ** A path step. For each input node: the axis nodes -> the node test ->
    ** the predicates (positions in axis order, for reverse axes — from the nearest).
    ** There is one axis buffer for the whole step; without predicates the matching
    ** nodes go straight into the result — without an intermediate list per input node.
    */
    std::vector<XPathItem> applyStep(const Step& step, const std::vector<XPathItem>& inputs,
        const EvalContext& context)
    {
        std::vector<XPathItem> result;
        std::vector<XPathItem> axis;
        std::vector<XPathItem> candidates;
        std::vector<XPathItem>* target = &candidates;

        if (step.predicates.empty())
            target = &result;
        for (const XPathItem& input : inputs)
        {
            axis.clear();
            candidates.clear();
            appendAxisItems(step.axis, input, axis);
            for (const XPathItem& item : axis)
            {
                if (matchesTest(step, item, context))
                    target->push_back(item);
            }
            if (step.predicates.empty())
                continue;
            candidates = applyPredicates(step.predicates, std::move(candidates), context);
            result.insert(result.end(), candidates.begin(), candidates.end());
        }
        orderStepResult(step.axis, inputs.size(), result);
        return result;
    }

    /*
    ** Предикат-число означает «позиция равна числу» ([2] = [position() = 2]),
    ** любой другой результат приводится к логическому значению. В
    ** утверждениях XSD число в предикате — точный xs:integer (DECIMAL).
    ** Переменные и источник типов внешнего контекста видны в предикате.
    **
    ** A number predicate means "the position equals the number" ([2] = [position() = 2]),
    ** any other result is converted to a boolean. In
    ** XSD assertions a number in a predicate is an exact xs:integer (DECIMAL).
    ** The variables and the type source of the outer context are visible in the predicate.
    */
    std::vector<XPathItem> applyPredicates(const std::vector<ExprPtr>& predicates,
        std::vector<XPathItem> items, const EvalContext& outer)
    {
        for (const ExprPtr& predicate : predicates)
        {
            std::vector<XPathItem> kept;
            for (std::size_t index = 0; index < items.size(); ++index)
            {
                EvalContext context{items[index], index + 1, items.size(), outer.user, outer.typed, outer.locals};
                XPathValue value = predicate->evaluate(context);
                bool keep = false;
                if (value.getType() == ValueType::NUMBER)
                    keep = value.toNumber() == static_cast<double>(index + 1);
                else if (value.getType() == ValueType::DECIMAL)
                    keep = utils::compareDecimals(value.getDecimal(),
                        utils::decimalFromInteger(static_cast<long long>(index + 1))) == 0;
                else
                    keep = value.toBoolean();
                if (keep)
                    kept.push_back(items[index]);
            }
            items = std::move(kept);
        }
        return items;
    }
}
