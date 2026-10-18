#include "xsd/internal/validation_run.hpp"

#include <string_view>
#include <unordered_map>

namespace beresta::xsd::detail
{
    // Детей не больше этого — номера считаются перебором пар, без хеш-таблицы
    // No more children than this — the numbers are computed by trying pairs, without a hash table
    static const std::size_t FEW_CHILDREN = 8;

    // Сегмент пути ребёнка с номером среди одноимённых соседей, если их несколько
    // A child path segment with the number among same-named siblings, if there are several
    std::string childPath(const std::string& parentPath, const Element& child)
    {
        std::size_t index = 0;
        std::size_t total = 0;

        for (const Node* node = child.getParent()->getFirstChild(); node != nullptr; node = node->getNextSibling())
        {
            if (node->getType() != NodeType::ELEMENT || static_cast<const Element*>(node)->getName() != child.getName())
                continue;
            ++total;
            if (node == &child)
                index = total;
        }
        std::string path = parentPath + "/" + child.getName();
        if (total > 1)
            path += "[" + std::to_string(index) + "]";
        return path;
    }

    /*
    ** Номера одноимённых детей перебором всех пар: O(n²), но без выделения
    ** памяти. У почти всех элементов детей единицы, и для них это быстрее
    ** хеш-таблицы, которую пришлось бы заводить для каждого элемента.
    **
    ** The numbers of same-named children by trying all pairs: O(n²), but without memory
    ** allocation. Almost all elements have only a few children, and for them this is faster
    ** than a hash table that would have to be created for every element.
    */
    static void numberFewChildren(const std::vector<const Element*>& children, std::size_t parent,
        std::vector<PathStep>& steps)
    {
        for (std::size_t index = 0; index < children.size(); ++index)
        {
            std::size_t ordinal = 0;
            std::size_t total = 0;

            for (std::size_t other = 0; other < children.size(); ++other)
            {
                if (children[other]->getName() != children[index]->getName())
                    continue;
                ++total;
                if (other <= index)
                    ++ordinal;
            }
            if (total == 1)
                ordinal = 0;
            steps.push_back(PathStep{parent, children[index], ordinal});
        }
    }

    /*
    ** Номера одноимённых детей за O(n) в два прохода: сначала число детей
    ** с каждым именем, затем номер каждого ребёнка среди них. Перебор пар
    ** у элемента со 100 тысячами детей занял бы десятки секунд.
    **
    ** The numbers of same-named children in O(n) in two passes: first the number of children
    ** with each name, then the number of each child among them. Trying pairs
    ** for an element with 100 thousand children would take tens of seconds.
    */
    static void numberManyChildren(const std::vector<const Element*>& children, std::size_t parent,
        std::vector<PathStep>& steps)
    {
        std::unordered_map<std::string_view, std::pair<std::size_t, std::size_t>> counters;

        for (const Element* child : children)
            ++counters[child->getName()].second;
        for (const Element* child : children)
        {
            std::pair<std::size_t, std::size_t>& counter = counters[child->getName()];
            std::size_t ordinal = 0;

            ++counter.first;
            if (counter.second > 1)
                ordinal = counter.first;
            steps.push_back(PathStep{parent, child, ordinal});
        }
    }

    // Шаги путей всех детей из _children подряд; возвращается номер шага первого ребёнка
    // The path steps of all children from _children in a row; the step number of the first child is returned
    std::size_t ValidationRun::addChildSteps(std::size_t parent)
    {
        std::size_t first = _steps.size();

        if (_children.elements.size() <= FEW_CHILDREN)
            numberFewChildren(_children.elements, parent, _steps);
        else
            numberManyChildren(_children.elements, parent, _steps);
        return first;
    }

    // Строка пути /a/b[2]/c: шаги собираются от элемента к корню, текст — от корня
    // The path string /a/b[2]/c: the steps are collected from the element to the root, the text from the root
    std::string ValidationRun::pathText(std::size_t step) const
    {
        std::vector<std::size_t> chain;
        std::string path;

        for (std::size_t current = step; current != NO_PARENT_STEP; current = _steps[current].parent)
            chain.push_back(current);
        for (auto position = chain.rbegin(); position != chain.rend(); ++position)
        {
            const PathStep& segment = _steps[*position];
            path += '/';
            path += segment.element->getName();
            if (segment.ordinal != 0)
                path += "[" + std::to_string(segment.ordinal) + "]";
        }
        return path;
    }
}
