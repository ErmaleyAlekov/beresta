#ifndef XSD_DATATYPE_CHECKS_HPP
#define XSD_DATATYPE_CHECKS_HPP

#include "xsd/simple_type.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace beresta::xsd::detail
{
    // Значение даты/времени, приведённое к сравнимому виду
    // A date/time value converted to a comparable form
    struct DateTimeValue
    {
        long long year = 0;
        int month = 1;
        int day = 1;
        int hour = 0;
        int minute = 0;
        long double second = 0.0L;
        bool hasTimezone = false;
        int timezoneMinutes = 0;
    };

    // ---- Пробелы (whitespace.cpp) ----
    // ---- Whitespace (whitespace.cpp) ----

    /*
    ** Значение после правила whiteSpace без лишней копии: само value, если
    ** оно уже нормально, иначе представление buffer с результатом.
    **
    ** The value after the whiteSpace rule without an extra copy: value itself if
    ** it is already normal, otherwise a view of buffer with the result.
    */
    std::string_view normalizedView(std::string_view value, WhiteSpace mode, std::string& buffer);

    // ---- Лексика чисел и двоичных данных (lexical_numbers.cpp) ----
    // ---- The lexis of numbers and binary data (lexical_numbers.cpp) ----
    bool isBooleanLexical(std::string_view value) noexcept;
    bool isDecimalLexical(std::string_view value, bool integerOnly) noexcept;
    bool isFloatLexical(std::string_view value) noexcept;
    std::optional<std::size_t> hexBinaryLength(std::string_view value) noexcept;
    std::optional<std::size_t> base64BinaryLength(std::string_view value) noexcept;

    // ---- Лексика строк (lexical_strings.cpp) ----
    // ---- The lexis of strings (lexical_strings.cpp) ----
    bool matchesLexicalForm(LexicalForm form, std::string_view value) noexcept;
    bool isLanguageLexical(std::string_view value) noexcept;
    bool isDurationLexical(std::string_view value) noexcept;
    bool isResolvableQName(std::string_view value, const Element* context);

    // ---- Даты (lexical_dates.cpp, lexical_date_parts.cpp) ----
    // ---- Dates (lexical_dates.cpp, lexical_date_parts.cpp) ----
    bool parseDateTimeValue(Primitive primitive, std::string_view value, DateTimeValue& result);
    bool readYear(std::string_view text, std::size_t& position, long long& year) noexcept;
    bool readDigits(std::string_view text, std::size_t& position, std::size_t count, int& value) noexcept;
    bool readSeconds(std::string_view text, std::size_t& position, long double& seconds) noexcept;
    bool readTimezone(std::string_view text, std::size_t& position, DateTimeValue& result) noexcept;

    // ---- Сравнение значений (value_compare.cpp) ----
    // ---- Value comparison (value_compare.cpp) ----

    // Десятичное число по частям: знак, целая часть без ведущих нулей, дробь без хвостовых
    // A decimal number in parts: the sign, the integer part without leading zeros, the fraction without trailing ones
    struct DecimalParts
    {
        bool negative = false;
        std::string_view integer;
        std::string_view fraction;
    };

    // ---- Значения (value_compare.cpp, value_order.cpp) ----
    // Разбор уже проверенной лексики decimal на части (части ссылаются на value)
    // ---- Values (value_compare.cpp, value_order.cpp) ----
    // Splits already checked decimal lexis into parts (the parts refer to value)
    DecimalParts splitDecimal(std::string_view value) noexcept;

    // float / double в long double: INF, -INF, NaN, мантисса и экспонента
    // float / double to long double: INF, -INF, NaN, the mantissa and the exponent
    long double parseFloatValue(std::string_view value) noexcept;

    // Числовой ключ порядка даты/времени с учётом часового пояса
    // A numeric order key of a date/time taking the time zone into account
    long double dateTimeKey(const DateTimeValue& value) noexcept;

    // Каноническая запись атомарного значения с префиксом примитива (value_canonical.cpp)
    // The canonical form of an atomic value with the primitive prefix (value_canonical.cpp)
    std::string canonicalAtomic(Primitive primitive, const std::string& value);

    // -1, 0, 1 — порядок; 2 — несравнимы (NaN, разные виды значений)
    // -1, 0, 1 — the order; 2 — incomparable (NaN, different kinds of values)
    int compareDecimal(std::string_view left, std::string_view right);
    int compareOrdered(Primitive primitive, std::string_view left, std::string_view right);

    // Нарушает ли результат сравнения границу (нижнюю/верхнюю, включающую/исключающую)
    // Whether the comparison result violates a bound (lower/upper, inclusive/exclusive)
    bool breaksBound(int order, bool lower, bool inclusive) noexcept;

    // Число значащих цифр и цифр дробной части десятичного числа
    // The number of significant digits and fraction digits of a decimal number
    void countDecimalDigits(std::string_view value, std::size_t& total, std::size_t& fraction);

    // ---- Проверка лексики и фасетов атомарного значения (facet_checks.cpp) ----
    // ---- Checking the lexis and facets of an atomic value (facet_checks.cpp) ----
    std::optional<std::string> checkLexical(const SimpleType& type, std::string_view value,
        const Element* context);
    std::optional<std::string> checkLengthFacets(const SimpleType& type, std::string_view value);
    std::optional<std::string> checkRangeFacets(const SimpleType& type, std::string_view value);
    std::optional<std::string> checkDigitFacets(const SimpleType& type, std::string_view value);
    std::optional<std::string> checkPatternsAndEnumeration(const SimpleType& type,
        std::string_view value);
}

#endif
