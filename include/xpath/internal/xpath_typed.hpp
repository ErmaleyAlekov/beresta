#ifndef XPATH_TYPED_HPP
#define XPATH_TYPED_HPP

#include "xpath/internal/xpath_ast.hpp"
#include "utils/decimal.hpp"

#include <cstddef>
#include <string_view>
#include <vector>

namespace beresta::xpath::detail
{
    // Число цифр дробной части у частного двух xs:decimal (точность в XPath 2.0 — на выбор реализации)
    // The number of fraction digits of the quotient of two xs:decimal values (the precision in XPath 2.0 is implementation-defined)
    inline constexpr std::size_t DECIMAL_DIVISION_SCALE = 18;

    /*
    ** TypedValueSource — источник типизированных значений узлов (аналог
    ** PSVI для модели данных XPath 2.0, паттерн Strategy). Его реализует
    ** модуль XSD: тип каждого узла известен после проверки, а разбор дат —
    ** часть типов данных XSD. Так XPath не зависит от XSD (инверсия
    ** зависимостей), а утверждения видят xs:decimal как точное число.
    **
    ** TypedValueSource — the source of typed values of nodes (an analog of the
    ** PSVI for the XPath 2.0 data model, the Strategy pattern). The XSD module
    ** implements it: the type of each node is known after validation, and date parsing is
    ** a part of the XSD datatypes. This way XPath does not depend on XSD (dependency
    ** inversion), and assertions see xs:decimal as an exact number.
    */
    class TypedValueSource
    {
    public:
        virtual ~TypedValueSource();

        /*
        ** Типизированное значение узла: DECIMAL, NUMBER (xs:double/float),
        ** BOOLEAN, DATE, DATE_TIME или STRING (строки и нетипизированные узлы).
        ** Элемент с содержимым только из элементов значения не имеет — ошибка.
        **
        ** The typed value of a node: DECIMAL, NUMBER (xs:double/float),
        ** BOOLEAN, DATE, DATE_TIME or STRING (strings and untyped nodes).
        ** An element with element-only content has no value — an error.
        */
        virtual XPathValue atomize(const XPathItem& item) const = 0;

        // Строка -> дата (type = DATE / DATE_TIME); false — запись не по лексике типа
        // A string -> a date (type = DATE / DATE_TIME); false — the notation does not follow the type lexis
        virtual bool castDate(ValueType type, std::string_view text, XPathValue& result) const = 0;
    };

    /*
    ** Источник без схемы (паттерн Null Object): все узлы — нетипизированные
    ** строки, даты не распознаются. Нужен, когда выражение-утверждение
    ** вычисляют напрямую, а не валидатором.
    **
    ** A source without a schema (the Null Object pattern): all nodes are untyped
    ** strings, dates are not recognized. It is needed when an assertion expression
    ** is evaluated directly rather than by the validator.
    */
    class UntypedValueSource : public TypedValueSource
    {
    public:
        XPathValue atomize(const XPathItem& item) const override;
        bool castDate(ValueType type, std::string_view text, XPathValue& result) const override;
    };

    // ---- Атомы (xpath_typed_atoms.cpp) ----
    // ---- Atoms (xpath_typed_atoms.cpp) ----

    // Атомы значения: множество — типизированные значения узлов, атом — он сам
    // The atoms of a value: a set — the typed values of the nodes, an atom — itself
    std::vector<XPathValue> atomizeValue(const XPathValue& value, const EvalContext& context);

    // Не больше одного атома: пустое множество — false; два и больше — ошибка
    // At most one atom: an empty set — false; two or more — an error
    bool singleAtom(const XPathValue& value, const EvalContext& context, XPathValue& atom);

    // Точное число из атома: decimal, строка-запись decimal, целое double; иначе false
    // An exact number from an atom: a decimal, a string with a decimal notation, an integer double; otherwise false
    bool exactDecimal(const XPathValue& atom, utils::Decimal& result);

    // Атом как double; строка, не являющаяся числом, — ошибка приведения
    // An atom as a double; a string that is not a number is a cast error
    double numericDouble(const XPathValue& atom);

    // Дата ли это (DATE или DATE_TIME)
    // Whether it is a date (DATE or DATE_TIME)
    bool isDateValue(const XPathValue& value) noexcept;

    // ---- Сравнения (xpath_typed_compare.cpp, xpath_typed_compare_atoms.cpp) ----
    // ---- Comparisons (xpath_typed_compare.cpp, xpath_typed_compare_atoms.cpp) ----

    // Сравнение двух атомов по правилам XPath 2.0 (op — общий оператор = != < <= > >=)
    // Comparing two atoms by the XPath 2.0 rules (op is a general operator = != < <= > >=)
    bool compareTypedAtoms(BinaryOperator op, const XPathValue& left, const XPathValue& right,
        const EvalContext& context);

    // Подходит ли порядок (-1, 0, 1) под оператор сравнения
    // Whether the order (-1, 0, 1) fits the comparison operator
    bool orderMatches(BinaryOperator op, int order) noexcept;

    // Сравнения, сравнения значений и арифметика типизированного режима
    // Comparisons, value comparisons and arithmetic of the typed mode
    XPathValue typedBinary(BinaryOperator op, const XPathValue& left, const XPathValue& right,
        const EvalContext& context);

    // ---- Арифметика (xpath_typed_arithmetic.cpp) ----
    // ---- Arithmetic (xpath_typed_arithmetic.cpp) ----
    XPathValue typedArithmetic(BinaryOperator op, const XPathValue& left, const XPathValue& right,
        const EvalContext& context);
    XPathValue typedNegate(const XPathValue& value, const EvalContext& context);

    // ---- Приведение типов (xpath_typed_cast.cpp) ----
    // ---- Casting (xpath_typed_cast.cpp) ----
    XPathValue castAtomic(AtomicType type, const XPathValue& value, const EvalContext& context);

    // Ошибка «значение нельзя привести к типу xs:имя»
    // The error "the value cannot be cast to the type xs:name"
    [[noreturn]] void failCast(const std::string& value, const char* typeName);
}

#endif
