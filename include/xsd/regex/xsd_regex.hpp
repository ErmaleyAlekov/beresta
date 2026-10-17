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
    enum class RegexKind
    {
        CHARACTERS,     // один символ из множества
        SEQUENCE,       // дети подряд
        ALTERNATION     // один из детей
    };

    /*
    ** RegexNode — разобранное выражение. Повторение хранится в самом узле
    ** (minimum..maximum), как в грамматике XSD: piece = atom quantifier?.
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
    inline constexpr std::size_t REGEX_UNBOUNDED = static_cast<std::size_t>(-1);

    /*
    ** Regex — регулярное выражение XML Schema (приложение F XSD 1.0).
    ** Компилируется в НКА Томпсона и проверяется моделированием всех
    ** состояний одновременно: время O(длина текста × число состояний),
    ** «катастрофический» возврат невозможен в принципе. Совпадение всегда
    ** с целой строкой (в XSD шаблон неявно привязан к началу и концу).
    ** Синтаксическая ошибка шаблона — DomException(SCHEMA_ERROR).
    */
    class Regex
    {
    public:
        explicit Regex(std::string_view pattern);

        // Соответствует ли ВСЯ строка шаблону
        bool matches(std::string_view text) const;

        const std::string& getPattern() const noexcept;

    private:
        // Состояние автомата: символ из множества, ε-развилка, ε-переход или приём
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
        struct Fragment
        {
            long start;
            std::vector<std::pair<long, int>> exits;
        };

        // ---- Компиляция (xsd_regex_compile.cpp) ----
        Fragment compileNode(const RegexNode& node);
        Fragment compileOnce(const RegexNode& node);
        Fragment compileSequence(const std::vector<RegexNode>& children);
        Fragment compileAlternation(const std::vector<RegexNode>& children);
        void connect(const Fragment& fragment, long target);

        // ---- Построение и шаги автомата (xsd_regex_build.cpp) ----
        void appendOptionalCopies(Fragment& result, const RegexNode& node);
        long addState(StateKind kind, std::size_t setIndex);
        void advanceStates(std::uint32_t code, const std::vector<long>& current,
            std::vector<long>& next, std::vector<std::size_t>& marks, std::size_t generation) const;
        bool acceptsAny(const std::vector<long>& states) const noexcept;

        // ---- Моделирование (xsd_regex_match.cpp) ----
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
    */
    class RegexParser
    {
    public:
        RegexParser(std::string_view pattern, std::vector<CharSet>& sets);

        RegexNode parse();

    private:
        // ---- Структура (xsd_regex_parser.cpp) ----
        RegexNode parseAlternation(std::size_t depth);
        RegexNode parseBranch(std::size_t depth);
        RegexNode parseAtom(std::size_t depth);

        // ---- Атомы и квантификаторы (xsd_regex_quantifier.cpp) ----
        RegexNode parseSimpleAtom();
        void parseQuantifier(RegexNode& node);
        void parseBraceQuantifier(RegexNode& node);
        std::size_t parseNumber();

        // ---- Классы символов (xsd_regex_classes.cpp) ----
        CharSet parseClassExpression();
        CharSet parseClassGroup();
        bool applySubtraction(CharSet& set, bool negated);
        void parseClassItem(CharSet& set);
        std::uint32_t parseRangeEnd();

        // ---- Экранирование (xsd_regex_escapes.cpp) ----
        CharSet parseEscape(bool& isSingle, std::uint32_t& single);
        CharSet parseProperty(bool complement);

        // ---- Символы (xsd_regex_chars.cpp) ----
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
