#include "xpath/internal/xpath_eval.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    /*
    ** Шаг пути. Для каждого входного узла: узлы оси -> проверка узла ->
    ** предикаты (позиции — в порядке оси, для обратных осей — от ближнего).
    ** Итог объединяется и упорядочивается в порядке документа.
    */
    std::vector<XPathItem> applyStep(const Step& step, const std::vector<XPathItem>& inputs,
        const XPathContext& user)
    {
        std::vector<XPathItem> result;

        for (const XPathItem& input : inputs)
        {
            std::vector<XPathItem> candidates;
            for (const XPathItem& item : axisItems(step.axis, input))
            {
                if (matchesTest(step, item, user))
                    candidates.push_back(item);
            }
            candidates = applyPredicates(step.predicates, std::move(candidates), user);
            result.insert(result.end(), candidates.begin(), candidates.end());
        }
        sortDocumentOrder(result);
        return result;
    }

    /*
    ** Предикат-число означает «позиция равна числу» ([2] = [position() = 2]),
    ** любой другой результат приводится к логическому значению.
    */
    std::vector<XPathItem> applyPredicates(const std::vector<ExprPtr>& predicates,
        std::vector<XPathItem> items, const XPathContext& user)
    {
        for (const ExprPtr& predicate : predicates)
        {
            std::vector<XPathItem> kept;
            for (std::size_t index = 0; index < items.size(); ++index)
            {
                EvalContext context{items[index], index + 1, items.size(), user};
                XPathValue value = predicate->evaluate(context);
                bool keep = value.toBoolean();
                if (value.getType() == ValueType::NUMBER)
                    keep = value.toNumber() == static_cast<double>(index + 1);
                if (keep)
                    kept.push_back(items[index]);
            }
            items = std::move(kept);
        }
        return items;
    }
}
