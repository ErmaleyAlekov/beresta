#ifndef XSD_DATATYPE_CHECKS_HPP
#define XSD_DATATYPE_CHECKS_HPP

#include "xsd/simple_type.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace beresta::xsd::detail
{
    // Значение даты/времени, приведённое к сравнимому виду
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

    // ---- Лексика чисел и двоичных данных (lexical_numbers.cpp) ----
    bool isBooleanLexical(std::string_view value) noexcept;
    bool isDecimalLexical(std::string_view value, bool integerOnly) noexcept;
    bool isFloatLexical(std::string_view value) noexcept;
    std::optional<std::size_t> hexBinaryLength(std::string_view value) noexcept;
    std::optional<std::size_t> base64BinaryLength(std::string_view value) noexcept;

    // ---- Лексика строк (lexical_strings.cpp) ----
    bool matchesLexicalForm(LexicalForm form, std::string_view value) noexcept;
    bool isLanguageLexical(std::string_view value) noexcept;
    bool isDurationLexical(std::string_view value) noexcept;
    bool isResolvableQName(std::string_view value, const Element* context);

    // ---- Даты (lexical_dates.cpp, lexical_date_parts.cpp) ----
    bool parseDateTimeValue(Primitive primitive, std::string_view value, DateTimeValue& result);
    bool readYear(std::string_view text, std::size_t& position, long long& year) noexcept;
    bool readDigits(std::string_view text, std::size_t& position, std::size_t count, int& value) noexcept;
    bool readSeconds(std::string_view text, std::size_t& position, long double& seconds) noexcept;
    bool readTimezone(std::string_view text, std::size_t& position, DateTimeValue& result) noexcept;

    // ---- Сравнение значений (value_compare.cpp) ----

    // -1, 0, 1 — порядок; 2 — несравнимы (NaN, разные виды значений)
    int compareDecimal(std::string_view left, std::string_view right);
    int compareOrdered(Primitive primitive, std::string_view left, std::string_view right);

    // Нарушает ли результат сравнения границу (нижнюю/верхнюю, включающую/исключающую)
    bool breaksBound(int order, bool lower, bool inclusive) noexcept;

    // Число значащих цифр и цифр дробной части десятичного числа
    void countDecimalDigits(std::string_view value, std::size_t& total, std::size_t& fraction);

    // ---- Проверка лексики и фасетов атомарного значения (facet_checks.cpp) ----
    std::optional<std::string> checkLexical(const SimpleType& type, std::string_view value,
        const Element* context);
    std::optional<std::string> checkLengthFacets(const SimpleType& type, std::string_view value);
    std::optional<std::string> checkRangeFacets(const SimpleType& type, std::string_view value);
    std::optional<std::string> checkDigitFacets(const SimpleType& type, std::string_view value);
    std::optional<std::string> checkPatternsAndEnumeration(const SimpleType& type,
        std::string_view value);
}

#endif
