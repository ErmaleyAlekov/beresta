#include "xsd/regex/xsd_regex.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta::xsd
{
    // Ограничение размера автомата: {1000}{1000} не должен съесть всю память
    // The automaton size limit: {1000}{1000} must not eat all the memory
    static constexpr std::size_t MAX_STATES = 100000;

    /*
    ** Необязательные копии: каждая начинается с ε-развилки «войти / пропустить».
    ** Для unbounded копия одна, а её выход ведёт обратно в развилку (цикл).
    **
    ** Optional copies: each starts with an ε-split "enter / skip".
    ** For unbounded there is one copy, and its exit leads back to the split (a loop).
    */
    void Regex::appendOptionalCopies(Fragment& result, const RegexNode& node)
    {
        std::size_t optional = node.maximum - node.minimum;

        if (node.maximum == REGEX_UNBOUNDED)
            optional = 1;
        for (std::size_t copy = 0; copy < optional; ++copy)
        {
            long split = addState(StateKind::SPLIT, 0);
            Fragment part = compileOnce(node);
            _states[static_cast<std::size_t>(split)].out = part.start;
            connect(result, split);
            result.exits = std::move(part.exits);
            if (node.maximum == REGEX_UNBOUNDED)
            {
                connect(result, split);
                result.exits.clear();
            }
            result.exits.emplace_back(split, 1);
        }
    }

    // Новое состояние автомата (с проверкой лимита размера)
    // A new automaton state (with the size limit check)
    long Regex::addState(StateKind kind, std::size_t setIndex)
    {
        if (_states.size() >= MAX_STATES)
            throw DomException(DomErrorCode::SCHEMA_ERROR,
                i18n::format(i18n::Message::PATTERN_TOO_LARGE, {_pattern}));
        _states.push_back(State{kind, setIndex, -1, -1});
        return static_cast<long>(_states.size() - 1);
    }

    // Один шаг моделирования: из каждого активного состояния — по символу code
    // One simulation step: from each active state — by the character code
    void Regex::advanceStates(std::uint32_t code, const std::vector<long>& current,
        std::vector<long>& next, std::vector<std::size_t>& marks, std::size_t generation) const
    {
        next.clear();
        for (long state : current)
        {
            const State& node = _states[static_cast<std::size_t>(state)];
            if (node.kind == StateKind::CONSUME && _sets[node.setIndex].contains(code))
                addReachable(node.out, next, marks, generation);
        }
    }

    // Есть ли среди активных состояние приёма
    // Whether there is an accepting state among the active ones
    bool Regex::acceptsAny(const std::vector<long>& states) const noexcept
    {
        for (long state : states)
        {
            if (_states[static_cast<std::size_t>(state)].kind == StateKind::ACCEPT)
                return true;
        }
        return false;
    }

    // Исходный шаблон
    // The source pattern
    const std::string& Regex::getPattern() const noexcept
    {
        return _pattern;
    }
}
