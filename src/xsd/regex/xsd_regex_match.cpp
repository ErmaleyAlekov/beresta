#include "xsd/regex/xsd_regex.hpp"
#include "utils/utf8.hpp"

#include <utility>

namespace beresta::xsd
{
    // Разбор шаблона и построение автомата с завершающим состоянием приёма
    // Parses the pattern and builds the automaton with a final accepting state
    Regex::Regex(std::string_view pattern)
        : _pattern(pattern), _sets(), _states(), _start(-1)
    {
        RegexNode root = RegexParser(pattern, _sets).parse();
        Fragment fragment = compileNode(root);

        connect(fragment, addState(StateKind::ACCEPT, 0));
        _start = fragment.start;
    }

    /*
    ** ε-замыкание без рекурсии: явный стек и отметки «поколения», чтобы
    ** каждое состояние попадало в список не более одного раза за шаг.
    **
    ** The ε-closure without recursion: an explicit stack and "generation" marks so that
    ** each state gets into the list at most once per step.
    */
    void Regex::addReachable(long state, std::vector<long>& list, std::vector<std::size_t>& marks,
        std::size_t generation) const
    {
        std::vector<long> stack{state};

        while (!stack.empty())
        {
            long current = stack.back();
            stack.pop_back();
            if (current < 0 || marks[static_cast<std::size_t>(current)] == generation)
                continue;
            marks[static_cast<std::size_t>(current)] = generation;
            const State& node = _states[static_cast<std::size_t>(current)];
            if (node.kind == StateKind::SPLIT)
                stack.push_back(node.alternative);
            if (node.kind == StateKind::SPLIT || node.kind == StateKind::EPSILON)
                stack.push_back(node.out);
            else
                list.push_back(current);
        }
    }

    /*
    ** Моделирование НКА: множество активных состояний продвигается на
    ** каждый символ текста. Строка подходит, если в конце активно
    ** состояние приёма.
    **
    ** The NFA simulation: the set of active states advances on
    ** each character of the text. The string matches if the accepting state
    ** is active at the end.
    */
    bool Regex::matches(std::string_view text) const
    {
        std::vector<std::size_t> marks(_states.size(), 0);
        std::vector<long> current;
        std::vector<long> next;
        std::size_t generation = 1;
        std::size_t index = 0;
        std::uint32_t code = 0;

        addReachable(_start, current, marks, generation);
        while (index < text.size() && !current.empty())
        {
            if (!utils::decodeUtf8Char(text, index, code))
                return false;
            advanceStates(code, current, next, marks, ++generation);
            current.swap(next);
        }
        return index >= text.size() && acceptsAny(current);
    }
}
