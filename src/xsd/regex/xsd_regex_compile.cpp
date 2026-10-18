#include "xsd/regex/xsd_regex.hpp"

#include <utility>

namespace beresta::xsd
{
    /*
    ** Повторение по Томпсону: minimum обязательных копий, затем либо
    ** «звезда» (unbounded), либо (maximum - minimum) необязательных копий.
    ** Каждая копия компилируется заново из дерева — так проще и надёжнее,
    ** чем копировать уже построенный подграф.
    **
    ** Thompson repetition: minimum mandatory copies, then either
    ** a "star" (unbounded), or (maximum - minimum) optional copies.
    ** Each copy is compiled anew from the tree — this is simpler and more reliable
    ** than copying an already built subgraph.
    */
    Regex::Fragment Regex::compileNode(const RegexNode& node)
    {
        Fragment result{addState(StateKind::EPSILON, 0), {}};

        result.exits.emplace_back(result.start, 0);
        for (std::size_t copy = 0; copy < node.minimum; ++copy)
        {
            Fragment part = compileOnce(node);
            connect(result, part.start);
            result.exits = std::move(part.exits);
        }
        appendOptionalCopies(result, node);
        return result;
    }

    // Узел без учёта его собственного повторения
    // A node without its own repetition
    Regex::Fragment Regex::compileOnce(const RegexNode& node)
    {
        if (node.kind == RegexKind::CHARACTERS)
        {
            long state = addState(StateKind::CONSUME, node.setIndex);
            return Fragment{state, {{state, 0}}};
        }
        if (node.kind == RegexKind::ALTERNATION)
            return compileAlternation(node.children);
        return compileSequence(node.children);
    }

    // Последовательность: выход каждого фрагмента ведёт во вход следующего
    // A sequence: the exit of each fragment leads to the entry of the next one
    Regex::Fragment Regex::compileSequence(const std::vector<RegexNode>& children)
    {
        Fragment result{addState(StateKind::EPSILON, 0), {}};

        result.exits.emplace_back(result.start, 0);
        for (const RegexNode& child : children)
        {
            Fragment part = compileNode(child);
            connect(result, part.start);
            result.exits = std::move(part.exits);
        }
        return result;
    }

    // Выбор: цепочка ε-развилок, выходы всех вариантов объединяются
    // A choice: a chain of ε-splits, the exits of all alternatives are merged
    Regex::Fragment Regex::compileAlternation(const std::vector<RegexNode>& children)
    {
        Fragment result{addState(StateKind::SPLIT, 0), {}};
        long split = result.start;

        for (std::size_t index = 0; index < children.size(); ++index)
        {
            Fragment part = compileNode(children[index]);
            result.exits.insert(result.exits.end(), part.exits.begin(), part.exits.end());
            _states[static_cast<std::size_t>(split)].out = part.start;
            if (index + 1 == children.size())
                break;
            long next = addState(StateKind::SPLIT, 0);
            _states[static_cast<std::size_t>(split)].alternative = next;
            split = next;
        }
        return result;
    }

    // «Висячие» выходы фрагмента направляются в target
    // The "dangling" exits of a fragment are directed to target
    void Regex::connect(const Fragment& fragment, long target)
    {
        for (const std::pair<long, int>& exit : fragment.exits)
        {
            State& state = _states[static_cast<std::size_t>(exit.first)];
            if (exit.second == 0)
                state.out = target;
            else
                state.alternative = target;
        }
    }
}
