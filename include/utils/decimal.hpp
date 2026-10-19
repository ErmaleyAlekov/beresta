#ifndef UTILS_DECIMAL_HPP
#define UTILS_DECIMAL_HPP

#include <cstddef>
#include <string>
#include <string_view>

namespace beresta::utils
{
    /*
    ** Decimal — точное десятичное число произвольной длины (значение
    ** xs:decimal). Значение = ±digits · 10^(−scale): digits — модуль без
    ** знака и без ведущих нулей (пустая строка — ноль), scale — число цифр
    ** дробной части. Число всегда нормализовано: у дробной части нет
    ** хвостовых нулей, у нуля нет знака, поэтому равные значения имеют
    ** равные поля. Арифметика идёт по десятичным цифрам столбиком, без
    ** double: 0.10 + 0.20 даёт ровно 0.3 — это и нужно банковским схемам.
    **
    ** Decimal — an exact decimal number of any length (an xs:decimal
    ** value). The value = ±digits · 10^(−scale): digits is the absolute value without
    ** a sign and without leading zeros (an empty string is zero), scale is the number of
    ** fraction digits. The number is always normalized: the fraction has no
    ** trailing zeros, zero has no sign, so equal values have
    ** equal fields. Arithmetic works on decimal digits by long hand, without
    ** double: 0.10 + 0.20 gives exactly 0.3 — this is what banking schemas need.
    */
    struct Decimal
    {
        bool negative = false;
        std::string digits;
        std::size_t scale = 0;
    };

    /*
    ** Десятичная запись по частям: знак, целая часть без ведущих нулей,
    ** дробная без хвостовых (части ссылаются на исходную строку).
    **
    ** A decimal notation in parts: the sign, the integer part without leading zeros,
    ** the fraction without trailing ones (the parts refer to the source string).
    */
    struct DecimalParts
    {
        bool negative = false;
        std::string_view integer;
        std::string_view fraction;
    };

    // ---- Текст (decimal_text.cpp, decimal_format.cpp) ----
    // ---- Text (decimal_text.cpp, decimal_format.cpp) ----

    // Лексика xs:decimal ([+-]цифры[.цифры], пробелы XML по краям); false — не число
    // The xs:decimal lexis ([+-]digits[.digits], XML spaces around); false — not a number
    bool readDecimal(std::string_view text, Decimal& result);

    // Целое число как Decimal
    // An integer as a Decimal
    Decimal decimalFromInteger(long long value);

    // double без потерь: только целые по модулю не больше 2^53; иначе false
    // A double without loss: only integers not greater than 2^53 in absolute value; otherwise false
    bool decimalFromDouble(double value, Decimal& result);

    // Каноническая запись: "-12.5", "3", "0" (без экспоненты и лишних нулей)
    // The canonical notation: "-12.5", "3", "0" (without an exponent and extra zeros)
    std::string formatDecimal(const Decimal& value);

    // Ближайшее double (для смешанной арифметики с xs:double)
    // The nearest double (for mixed arithmetic with xs:double)
    double decimalToDouble(const Decimal& value) noexcept;

    // ---- Сравнение и арифметика (decimal_normalize.cpp, decimal_arithmetic.cpp) ----
    // ---- Comparison and arithmetic (decimal_normalize.cpp, decimal_arithmetic.cpp) ----

    // -1, 0 или 1
    // -1, 0 or 1
    int compareDecimals(const Decimal& left, const Decimal& right);
    Decimal negateDecimal(const Decimal& value);
    Decimal addDecimals(const Decimal& left, const Decimal& right);
    Decimal subtractDecimals(const Decimal& left, const Decimal& right);
    Decimal multiplyDecimals(const Decimal& left, const Decimal& right);

    // ---- Деление и округление (decimal_division.cpp, decimal_rounding.cpp) ----
    // ---- Division and rounding (decimal_division.cpp, decimal_rounding.cpp) ----

    // Частное с scale цифрами дробной части (лишние отбрасываются); false — деление на ноль
    // The quotient with scale fraction digits (the extra ones are dropped); false — division by zero
    bool divideDecimals(const Decimal& left, const Decimal& right, std::size_t scale, Decimal& result);

    // Остаток со знаком делимого: left − right · trunc(left / right); false — деление на ноль
    // The remainder with the sign of the dividend: left − right · trunc(left / right); false — division by zero
    bool moduloDecimals(const Decimal& left, const Decimal& right, Decimal& result);

    Decimal truncateDecimal(const Decimal& value);
    Decimal floorDecimal(const Decimal& value);
    Decimal ceilingDecimal(const Decimal& value);

    // round() XPath: к ближайшему целому, половина — вверх (round(-2.5) = -2)
    // XPath round(): to the nearest integer, a half goes up (round(-2.5) = -2)
    Decimal roundDecimal(const Decimal& value);

    // ---- Сравнение записей без разбора в Decimal (decimal_text_compare.cpp) ----
    // ---- Comparing notations without parsing into a Decimal (decimal_text_compare.cpp) ----

    // Разбор уже проверенной лексики decimal на части (без выделения памяти)
    // Splits already checked decimal lexis into parts (without allocating memory)
    DecimalParts splitDecimal(std::string_view value) noexcept;

    // Точное сравнение двух проверенных записей decimal любой длины: -1, 0 или 1
    // An exact comparison of two checked decimal notations of any length: -1, 0 or 1
    int compareDecimalText(std::string_view left, std::string_view right) noexcept;
}

#endif
