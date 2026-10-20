#ifndef XSD_REGEX_HPP
#define XSD_REGEX_HPP

#include "xsd/regex/char_set.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::xsd
{
    // Узел дерева регулярного выражения
    // A node of the regular expression tree
    enum class RegexKind
    {
        CHARACTERS,     // один символ из множества / one character from the set
        SEQUENCE,       // дети подряд / the children in a row
        ALTERNATION     // один из детей / one of the children
    };

    /*
    ** RegexNode — разобранное выражение. Повторение хранится в самом узле
    ** (minimum..maximum), как в грамматике XSD: piece = atom quantifier?.
    **
    ** RegexNode — a parsed expression. The repetition is stored in the node itself
    ** (minimum..maximum), as in the XSD grammar: piece = atom quantifier?.
    */
    struct RegexNode
    {
        RegexKind kind = RegexKind::SEQUENCE;
        std::size_t setIndex = 0;
        std::size_t minimum = 1;
        std::size_t maximum = 1;
        std::vector<RegexNode> children;
    };

    // Значение «без верхней границы» для '*', '+' и {n,}
    // The "no upper bound" value for '*', '+' and {n,}
    inline constexpr std::size_t REGEX_UNBOUNDED = static_cast<std::size_t>(-1);

    /*
    ** Regex — регулярное выражение XML Schema (приложение F XSD 1.0).
    ** Компилируется в НКА Томпсона и проверяется моделированием всех
    ** состояний одновременно: время O(длина текста × число состояний),
    ** «катастрофический» возврат невозможен в принципе. Совпадение всегда
    ** с целой строкой (в XSD шаблон неявно привязан к началу и концу).
    ** Синтаксическая ошибка шаблона — DomException(SCHEMA_ERROR).
    **
    ** Regex — an XML Schema regular expression (appendix F of XSD 1.0).
    ** It is compiled into a Thompson NFA and matched by simulating all
    ** states at once: time O(text length × number of states),
    ** "catastrophic" backtracking is impossible in principle. The match is always
    ** against the whole string (in XSD a pattern is implicitly anchored at the start and the end).
    ** A pattern syntax error is DomException(SCHEMA_ERROR).
    */
    class Regex
    {
    public:
        explicit Regex(std::string_view pattern);

        // Соответствует ли ВСЯ строка шаблону
        // Whether the WHOLE string matches the pattern
        bool matches(std::string_view text) const;

        const std::string& getPattern() const noexcept;

    private:
        // Состояние автомата: символ из множества, ε-развилка, ε-переход или приём
        // An automaton state: a character from a set, an ε-split, an ε-transition or acceptance
        enum class StateKind
        {
            CONSUME, SPLIT, EPSILON, ACCEPT
        };

        struct State
        {
            StateKind kind;
            std::size_t setIndex;
            long out;
            long alternative;
        };

        // Фрагмент автомата: вход и «висячие» выходы (состояние, номер выхода)
        // An automaton fragment: the entry and the "dangling" exits (the state, the exit number)
        struct Fragment
        {
            long start;
            std::vector<std::pair<long, int>> exits;
        };

        // ---- Компиляция (xsd_regex_compile.cpp) ----
        // ---- Compilation (xsd_regex_compile.cpp) ----
        Fragment compileNode(const RegexNode& node);
        Fragment compileOnce(const RegexNode& node);
        Fragment compileSequence(const std::vector<RegexNode>& children);
        Fragment compileAlternation(const std::vector<RegexNode>& children);
        void connect(const Fragment& fragment, long target);

        // ---- Построение и шаги автомата (xsd_regex_build.cpp) ----
        // ---- Building and stepping the automaton (xsd_regex_build.cpp) ----
        void appendOptionalCopies(Fragment& result, const RegexNode& node);
        long addState(StateKind kind, std::size_t setIndex);
        void advanceStates(std::uint32_t code, const std::vector<long>& current,
            std::vector<long>& next, std::vector<std::size_t>& marks, std::size_t generation) const;
        bool acceptsAny(const std::vector<long>& states) const noexcept;

        // ---- Моделирование (xsd_regex_match.cpp) ----
        // ---- Simulation (xsd_regex_match.cpp) ----
        void addReachable(long state, std::vector<long>& list, std::vector<std::size_t>& marks,
            std::size_t generation) const;

        std::string _pattern;
        std::vector<CharSet> _sets;
        std::vector<State> _states;
        long _start;
    };

    /*
    ** RegexParser — рекурсивный спуск по грамматике регулярных выражений XSD:
    ** regExp = branch ('|' branch)*, branch = piece*, piece = atom quantifier?
    **
    ** RegexParser — recursive descent over the XSD regular expression grammar:
    ** regExp = branch ('|' branch)*, branch = piece*, piece = atom quantifier?
    */
    class RegexParser
    {
    public:
        RegexParser(std::string_view pattern, std::vector<CharSet>& sets);

        RegexNode parse();

    private:
        // ---- Структура (xsd_regex_parser.cpp) ----
        // ---- Structure (xsd_regex_parser.cpp) ----
        RegexNode parseAlternation(std::size_t depth);
        RegexNode parseBranch(std::size_t depth);
        RegexNode parseAtom(std::size_t depth);

        // ---- Атомы и квантификаторы (xsd_regex_quantifier.cpp) ----
        // ---- Atoms and quantifiers (xsd_regex_quantifier.cpp) ----
        RegexNode parseSimpleAtom();
        void parseQuantifier(RegexNode& node);
        void parseBraceQuantifier(RegexNode& node);
        std::size_t parseNumber();

        // ---- Классы символов (xsd_regex_classes.cpp) ----
        // ---- Character classes (xsd_regex_classes.cpp) ----
        CharSet parseClassExpression();
        CharSet parseClassGroup();
        bool applySubtraction(CharSet& set, bool negated);
        void parseClassItem(CharSet& set);
        std::uint32_t parseRangeEnd(std::uint32_t first);

        // ---- Экранирование (xsd_regex_escapes.cpp) ----
        // ---- Escapes (xsd_regex_escapes.cpp) ----
        CharSet parseEscape(bool& isSingle, std::uint32_t& single);
        CharSet parseProperty(bool complement);

        // ---- Символы (xsd_regex_chars.cpp) ----
        // ---- Characters (xsd_regex_chars.cpp) ----
        bool atEnd() const noexcept;
        std::uint32_t peekCode() const noexcept;
        std::uint32_t takeCode();
        RegexNode makeSetNode(CharSet set);
        [[noreturn]] void fail(const std::string& message) const;

        std::string_view _pattern;
        std::size_t _position;
        std::vector<CharSet>& _sets;
    };
}

#endif
