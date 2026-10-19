#ifndef UTILS_DECIMAL_DIGITS_HPP
#define UTILS_DECIMAL_DIGITS_HPP

#include "utils/decimal.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace beresta::utils::detail
{
    /*
    ** Действия над модулями — строками десятичных цифр (старшая цифра
    ** первой). Это «школьная» арифметика столбиком: она медленнее
    ** двоичной, но точна при любой длине, а суммы в документах короткие.
    ** Ведущие нули на входе допустимы, результат — без них ("" — ноль).
    **
    ** Operations on absolute values — strings of decimal digits (the most significant
    ** digit first). This is "school" long-hand arithmetic: it is slower than
    ** binary arithmetic but exact at any length, and the sums in documents are short.
    ** Leading zeros on input are allowed, the result has none ("" is zero).
    */

    // ---- Сравнение и действия столбиком (decimal_digits.cpp) ----
    // ---- Comparison and long-hand operations (decimal_digits.cpp) ----
    int compareDigits(std::string_view left, std::string_view right) noexcept;
    std::string addDigits(std::string_view left, std::string_view right);

    // left − right при left >= right
    // left − right when left >= right
    std::string subtractDigits(std::string_view left, std::string_view right);
    std::string multiplyDigits(std::string_view left, std::string_view right);

    // ---- Деление столбиком (decimal_long_division.cpp) ----
    // ---- Long division (decimal_long_division.cpp) ----

    // Целочисленное деление столбиком: частное и остаток (divisor не ноль)
    // Integer long division: the quotient and the remainder (divisor is not zero)
    std::string divideDigits(std::string_view dividend, std::string_view divisor, std::string& remainder);

    // Без ведущих нулей
    // Without leading zeros
    std::string_view stripLeadingZeros(std::string_view digits) noexcept;

    // ---- Нормализация (decimal_normalize.cpp) ----
    // ---- Normalization (decimal_normalize.cpp) ----

    // Нормализованное число из знака, цифр (возможно, с лишними нулями) и масштаба
    // A normalized number from a sign, digits (possibly with extra zeros) and a scale
    Decimal makeDecimal(bool negative, std::string digits, std::size_t scale);

    // Цифры числа, приведённые к большему масштабу scale (дописываются нули)
    // The digits of the number brought to a larger scale (zeros are appended)
    std::string scaledDigits(const Decimal& value, std::size_t scale);
}

#endif
