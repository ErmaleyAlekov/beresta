#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "support/parse_helpers.hpp"

namespace
{
    // Числа: границы целых типов, десятичные и с плавающей точкой
    void testNumericTypes()
    {
        TEST_ASSERT(support::acceptsBuiltin("int", "2147483647") && !support::acceptsBuiltin("int", "2147483648"));
        TEST_ASSERT(support::acceptsBuiltin("unsignedByte", "255") && !support::acceptsBuiltin("unsignedByte", "-1"));
        TEST_ASSERT(support::acceptsBuiltin("long", "-9223372036854775808") && !support::acceptsBuiltin("long", "9223372036854775808"));
        TEST_ASSERT(support::acceptsBuiltin("positiveInteger", "1") && !support::acceptsBuiltin("positiveInteger", "0"));
        TEST_ASSERT(support::acceptsBuiltin("decimal", "-12.50") && support::acceptsBuiltin("decimal", ".5") && !support::acceptsBuiltin("decimal", "1,5"));
        TEST_ASSERT(support::acceptsBuiltin("integer", " 42 ") && !support::acceptsBuiltin("integer", "4.2"));
        TEST_ASSERT(support::acceptsBuiltin("double", "1.5E-3") && support::acceptsBuiltin("float", "INF") && !support::acceptsBuiltin("double", "1e"));
        TEST_ASSERT(support::acceptsBuiltin("boolean", "1") && !support::acceptsBuiltin("boolean", "yes"));
    }

    // Даты, время, длительности и часовые пояса
    void testDateTypes()
    {
        TEST_ASSERT(support::acceptsBuiltin("date", "2024-02-29") && !support::acceptsBuiltin("date", "2023-02-29"));
        TEST_ASSERT(support::acceptsBuiltin("dateTime", "2026-10-17T12:30:00.5+03:00") && !support::acceptsBuiltin("dateTime", "2026-10-17"));
        TEST_ASSERT(support::acceptsBuiltin("time", "24:00:00") && !support::acceptsBuiltin("time", "24:00:01"));
        TEST_ASSERT(support::acceptsBuiltin("gYearMonth", "2026-10") && support::acceptsBuiltin("gMonthDay", "--02-29"));
        TEST_ASSERT(support::acceptsBuiltin("duration", "P1Y2M3DT4H5M6.5S") && support::acceptsBuiltin("duration", "-PT1H"));
        TEST_ASSERT(!support::acceptsBuiltin("duration", "P") && !support::acceptsBuiltin("duration", "P1S") && !support::acceptsBuiltin("duration", "PT"));
    }

    // Строковые формы, двоичные данные и списки
    void testStringAndBinaryTypes()
    {
        TEST_ASSERT(support::acceptsBuiltin("language", "ru-RU") && !support::acceptsBuiltin("language", "русский"));
        TEST_ASSERT(support::acceptsBuiltin("NCName", "книга") && !support::acceptsBuiltin("NCName", "a:b"));
        TEST_ASSERT(support::acceptsBuiltin("hexBinary", "0FB7") && !support::acceptsBuiltin("hexBinary", "0FB"));
        TEST_ASSERT(support::acceptsBuiltin("base64Binary", "SGVsbG8=") && !support::acceptsBuiltin("base64Binary", "SGVsbG8"));
        TEST_ASSERT(support::acceptsBuiltin("NMTOKENS", "a b c") && !support::acceptsBuiltin("NMTOKENS", "   "));
        TEST_ASSERT(support::acceptsBuiltin("token", "  много   пробелов  "));
        TEST_ASSERT(!support::acceptsBuiltin("QName", "p:name") && support::acceptsBuiltin("QName", "name"));
    }

    // Сравнение значений: 1.0 = 1 для decimal, порядок дат с учётом часового пояса
    void testValueEquality()
    {
        const beresta::xsd::SimpleType& decimal = *beresta::xsd::builtinType("decimal");
        const beresta::xsd::SimpleType& dateTime = *beresta::xsd::builtinType("dateTime");

        TEST_ASSERT(beresta::xsd::valuesEqual(decimal, "1.0", "001"));
        TEST_ASSERT(!beresta::xsd::valuesEqual(decimal, "1.01", "1.1"));
        TEST_ASSERT(beresta::xsd::valuesEqual(dateTime, "2026-01-01T03:00:00+03:00", "2026-01-01T00:00:00Z"));
    }
}

namespace suites
{
    // Регистрация тестов встроенных типов XSD
    void registerXsdDatatypeTests(test::TestRunner& runner)
    {
        runner.add("XSD: числовые типы", testNumericTypes);
        runner.add("XSD: даты и длительности", testDateTypes);
        runner.add("XSD: строки, двоичные данные, списки", testStringAndBinaryTypes);
        runner.add("XSD: равенство значений", testValueEquality);
    }
}
