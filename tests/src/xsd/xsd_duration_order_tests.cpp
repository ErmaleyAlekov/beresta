#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"
#include "xsd/internal/datatype_checks.hpp"

#include <string>

namespace
{
    const char* const XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"minY\" type=\"minYType\"/>"
        "<xs:element name=\"maxD\" type=\"maxDType\"/>"
        "<xs:element name=\"lowX\" type=\"lowXType\"/>"
        "<xs:element name=\"nan\" type=\"nanType\"/>"
        "<xs:element name=\"negative\"><xs:simpleType><xs:restriction base=\"xs:duration\">"
        "<xs:minInclusive value=\"-P2D\"/><xs:maxInclusive value=\"-P1D\"/>"
        "</xs:restriction></xs:simpleType></xs:element>"
        "<xs:simpleType name=\"minYType\">"
        "<xs:restriction base=\"xs:duration\">"
        "<xs:minInclusive value=\"P1Y\"/>"
        "</xs:restriction>"
        "</xs:simpleType>"
        "<xs:simpleType name=\"maxDType\">"
        "<xs:restriction base=\"xs:duration\">"
        "<xs:maxInclusive value=\"P30D\"/>"
        "</xs:restriction>"
        "</xs:simpleType>"
        "<xs:simpleType name=\"lowXType\">"
        "<xs:restriction base=\"xs:duration\">"
        "<xs:minExclusive value=\"PT1H\"/>"
        "</xs:restriction>"
        "</xs:simpleType>"
        "<xs:simpleType name=\"nanType\">"
        "<xs:restriction base=\"xs:double\">"
        "<xs:minInclusive value=\"0.0\"/>"
        "</xs:restriction>"
        "</xs:simpleType>"
        "</xs:schema>";

    /*
    ** Тесты для проверки частичного порядка xs:duration с фасетами-границами.
    ** Тест 1: testComparableDurations проверяет сравнимые значения.
    ** Тест 2: testIncomparableDurations проверяет неприводимые значения.
    ** Тест 3: testNaNBreaksBound проверяет NaN.
    **
    ** Tests for checking the partial order of xs:duration with boundary facets.
    ** Test 1: testComparableDurations checks comparable values.
    ** Test 2: testIncomparableDurations checks incomparable values.
    ** Test 3: testNaNBreaksBound checks NaN.
    */
    void testComparableDurations()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<minY>P1Y1D</minY>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<minY>P12M</minY>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<minY>P11M</minY>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<minY>-P2Y</minY>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<lowX>PT61M</lowX>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<lowX>PT3600S</lowX>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<negative>-PT48H</negative>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<negative>-PT24H</negative>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<negative>-PT49H</negative>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<negative>-PT23H</negative>").size() == 1);
    }

    void testIncomparableDurations()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<maxD>P1M</maxD>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<maxD>P29D</maxD>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<maxD>P31D</maxD>").size() == 1);
    }

    void testNaNBreaksBound()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<nan>NaN</nan>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<nan>1.5</nan>").empty());
    }

    // Точный порядок больших и дробных длительностей и отрицательного цикла календаря.
    // Exact ordering for large and fractional durations and a negative calendar cycle.
    void testExactDurationOrder()
    {
        using beresta::xsd::detail::compareDurations;
        const std::string prefix = "PT" + std::string(4999, '9');
        const std::string smaller = prefix + "8S";
        const std::string larger = prefix + "9S";

        TEST_ASSERT(compareDurations(smaller, larger) == -1);
        TEST_ASSERT(compareDurations(larger, smaller) == 1);
        TEST_ASSERT(compareDurations(larger, larger) == 0);
        TEST_ASSERT(compareDurations("PT24H", "P1D") == 0);
        TEST_ASSERT(compareDurations("-P400Y", "-P146097D") == 0);
        TEST_ASSERT(compareDurations("-P400YT1S", "-P146097D") == -1);
        TEST_ASSERT(compareDurations("PT0.00000000000000000000000000000000000001S", "PT0S") == 1);
    }
}

namespace suites
{
    // Регистрация тестов для xs:duration с фасетами-границами
    // Registers the tests for xs:duration with boundary facets
    void registerXsdDurationOrderTests(test::TestRunner& runner)
    {
        runner.add("XSD: сравнимые значения xs:duration", testComparableDurations);
        runner.add("XSD: неприводимые значения xs:duration", testIncomparableDurations);
        runner.add("XSD: NaN и границы xs:duration", testNaNBreaksBound);
        runner.add("XSD: точный порядок xs:duration", testExactDurationOrder);
    }
}
