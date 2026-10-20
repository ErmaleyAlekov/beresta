#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    const char* const XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"minY\" type=\"minYType\"/>"
        "<xs:element name=\"maxD\" type=\"maxDType\"/>"
        "<xs:element name=\"lowX\" type=\"lowXType\"/>"
        "<xs:element name=\"nan\" type=\"nanType\"/>"
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
    }
}
