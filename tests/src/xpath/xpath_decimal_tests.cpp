#include "suites/test_suites.hpp"
#include "utils/decimal.hpp"

#include <string>

namespace
{
    // Число из записи (запись обязана быть корректной)
    // A number from a notation (the notation must be valid)
    beresta::utils::Decimal number(const std::string& text)
    {
        beresta::utils::Decimal result;

        TEST_ASSERT(beresta::utils::readDecimal(text, result));
        return result;
    }

    /*
    ** Разбор и каноническая запись: знак, ведущие и хвостовые нули,
    ** пробелы по краям, "5." и ".5"; ноль без знака; ошибки лексики.
    ** Сравнение не зависит от записи (0.30 = 0.3, 1.10 = 1.1).
    **
    ** Parsing and the canonical notation: the sign, leading and trailing zeros,
    ** spaces around, "5." and ".5"; zero without a sign; lexis errors.
    ** The comparison does not depend on the notation (0.30 = 0.3, 1.10 = 1.1).
    */
    void testDecimalText()
    {
        beresta::utils::Decimal ignored;
        beresta::utils::Decimal integral;

        TEST_ASSERT_EQ(beresta::utils::formatDecimal(number("  -0012.3400 ")), "-12.34");
        TEST_ASSERT_EQ(beresta::utils::formatDecimal(number("+.5")), "0.5");
        TEST_ASSERT_EQ(beresta::utils::formatDecimal(number("5.")), "5");
        TEST_ASSERT_EQ(beresta::utils::formatDecimal(number("-0.000")), "0");
        TEST_ASSERT_EQ(beresta::utils::formatDecimal(number("0.007")), "0.007");
        TEST_ASSERT(!beresta::utils::readDecimal("", ignored) && !beresta::utils::readDecimal(".", ignored));
        TEST_ASSERT(!beresta::utils::readDecimal("1.2.3", ignored) && !beresta::utils::readDecimal("1e3", ignored));
        TEST_ASSERT(!beresta::utils::readDecimal("--1", ignored) && !beresta::utils::readDecimal("1 2", ignored));
        TEST_ASSERT(beresta::utils::compareDecimals(number("0.30"), number("0.3")) == 0);
        TEST_ASSERT(beresta::utils::compareDecimals(number("-1"), number("-0.5")) < 0);
        TEST_ASSERT(beresta::utils::compareDecimals(number("10"), number("9.99")) > 0);
        TEST_ASSERT(beresta::utils::compareDecimalText("1.10", "1.1") == 0);
        TEST_ASSERT(beresta::utils::decimalFromDouble(-3.0, integral) && beresta::utils::formatDecimal(integral) == "-3");
        TEST_ASSERT(!beresta::utils::decimalFromDouble(0.5, ignored) && !beresta::utils::decimalFromDouble(1e300, ignored));
        TEST_ASSERT(beresta::utils::decimalToDouble(number("-0.25")) == -0.25);
    }

    /*
    ** Сложение, вычитание, умножение — точно при любой длине: 0.1 + 0.2 =
    ** 0.3, перенос через 21 разряд, знаки, ноль без знака, произведение
    ** 30-значного числа.
    **
    ** Addition, subtraction, multiplication — exact at any length: 0.1 + 0.2 =
    ** 0.3, a carry through 21 places, the signs, zero without a sign, the product
    ** of a 30-digit number.
    */
    void testDecimalArithmetic()
    {
        using beresta::utils::formatDecimal;

        TEST_ASSERT_EQ(formatDecimal(beresta::utils::addDecimals(number("0.1"), number("0.2"))), "0.3");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::addDecimals(number("999999999999999999999.99"), number("0.01"))),
            "1000000000000000000000");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::addDecimals(number("-5"), number("3"))), "-2");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::subtractDecimals(number("3"), number("5.25"))), "-2.25");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::subtractDecimals(number("1.005"), number("1.005"))), "0");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::subtractDecimals(number("-0.3"), number("-0.1"))), "-0.2");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::multiplyDecimals(number("1.15"), number("2"))), "2.3");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::multiplyDecimals(number("-0.5"), number("0.5"))), "-0.25");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::multiplyDecimals(number("123456789012345678901234567890"),
            number("0.1"))), "12345678901234567890123456789");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::multiplyDecimals(number("-7"), number("0"))), "0");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::negateDecimal(number("0"))), "0");
    }

    /*
    ** Деление (с заданным числом знаков, лишние отбрасываются), остаток со
    ** знаком делимого, деление на ноль — false; floor, ceiling, round
    ** (половина — вверх) и усечение.
    **
    ** Division (with a given number of digits, the extra ones are dropped), the remainder with
    ** the sign of the dividend, division by zero — false; floor, ceiling, round
    ** (a half goes up) and truncation.
    */
    void testDecimalDivision()
    {
        using beresta::utils::formatDecimal;
        beresta::utils::Decimal result;

        TEST_ASSERT(beresta::utils::divideDecimals(number("10"), number("4"), 18, result) && formatDecimal(result) == "2.5");
        TEST_ASSERT(beresta::utils::divideDecimals(number("1"), number("3"), 5, result) && formatDecimal(result) == "0.33333");
        TEST_ASSERT(beresta::utils::divideDecimals(number("-7"), number("0.2"), 18, result) && formatDecimal(result) == "-35");
        TEST_ASSERT(!beresta::utils::divideDecimals(number("1"), number("0.00"), 18, result));
        TEST_ASSERT(beresta::utils::moduloDecimals(number("7"), number("3"), result) && formatDecimal(result) == "1");
        TEST_ASSERT(beresta::utils::moduloDecimals(number("-7"), number("3"), result) && formatDecimal(result) == "-1");
        TEST_ASSERT(beresta::utils::moduloDecimals(number("7.5"), number("2"), result) && formatDecimal(result) == "1.5");
        TEST_ASSERT(!beresta::utils::moduloDecimals(number("7"), number("0"), result));
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::floorDecimal(number("-0.5"))), "-1");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::ceilingDecimal(number("-0.5"))), "0");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::ceilingDecimal(number("2.01"))), "3");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::roundDecimal(number("2.5"))), "3");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::roundDecimal(number("-2.5"))), "-2");
        TEST_ASSERT_EQ(formatDecimal(beresta::utils::truncateDecimal(number("-1.9"))), "-1");
    }
}

namespace suites
{
    // Регистрация тестов точной десятичной арифметики (utils::Decimal)
    // Registers the tests of exact decimal arithmetic (utils::Decimal)
    void registerDecimalTests(test::TestRunner& runner)
    {
        runner.add("Decimal: разбор, запись и сравнение", testDecimalText);
        runner.add("Decimal: сложение, вычитание, умножение", testDecimalArithmetic);
        runner.add("Decimal: деление, остаток и округление", testDecimalDivision);
    }
}
