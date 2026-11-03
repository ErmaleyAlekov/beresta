#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Схема с элементом root, содержащим xs:sequence из необязательных элементов:
    ** Flag — complexType/simpleContent/extension base=xs:string с атрибутом type (xs:string)
    ** и <xs:assert test="$value = 'true' or $value = 'false'"/>; N — simpleContent
    ** extension base=xs:integer с <xs:assert test="$value gt 10"/>; Box — complexType
    ** с sequence из элемента item (xs:string, minOccurs=0) и <xs:assert test="empty($value)"/>.
    **
    ** Schema with a root element containing xs:sequence of optional elements:
    ** Flag — complexType/simpleContent/extension base=xs:string with an attribute type (xs:string)
    ** and <xs:assert test="$value = 'true' or $value = 'false'"/>; N — simpleContent
    ** extension base=xs:integer with <xs:assert test="$value gt 10"/>; Box — complexType
    ** with a sequence of an item element (xs:string, minOccurs=0) and <xs:assert test="empty($value)"/>.
    */
    const char* const XSD_ASSERT_VALUE =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"root\">"
        "<xs:complexType>"
        "<xs:sequence>"
        "<xs:element name=\"Flag\" minOccurs=\"0\">"
        "<xs:complexType>"
        "<xs:simpleContent>"
        "<xs:extension base=\"xs:string\">"
        "<xs:attribute name=\"type\" type=\"xs:string\"/>"
        "<xs:assert test=\"$value = 'true' or $value = 'false'\"/>"
        "</xs:extension>"
        "</xs:simpleContent>"
        "</xs:complexType>"
        "</xs:element>"
        "<xs:element name=\"N\" minOccurs=\"0\">"
        "<xs:complexType>"
        "<xs:simpleContent>"
        "<xs:extension base=\"xs:integer\">"
        "<xs:assert test=\"$value gt 10\"/>"
        "</xs:extension>"
        "</xs:simpleContent>"
        "</xs:complexType>"
        "</xs:element>"
        "<xs:element name=\"Box\" minOccurs=\"0\">"
        "<xs:complexType>"
        "<xs:sequence>"
        "<xs:element name=\"item\" type=\"xs:string\" minOccurs=\"0\"/>"
        "</xs:sequence>"
        "<xs:assert test=\"empty($value)\"/>"
        "</xs:complexType>"
        "</xs:element>"
        "</xs:sequence>"
        "</xs:complexType>"
        "</xs:element>"
        "</xs:schema>";

    /*
    ** Тесты для проверки значений элементов с xs:assert в сложных типах.
    ** Тесты проверяют корректность работы утверждений для различных значений элементов.
    **
    ** Tests for checking values of elements with xs:assert in complex types.
    ** Tests check the correctness of assertions for different element values.
    */
    void testSimpleContentValue()
    {
        std::vector<beresta::Diagnostic> errors = support::agreedSchemaErrors(XSD_ASSERT_VALUE,
            "<root><Flag type=\"boolean\">true</Flag></root>");
        TEST_ASSERT(errors.empty());

        errors = support::agreedSchemaErrors(XSD_ASSERT_VALUE,
            "<root><Flag>maybe</Flag></root>");
        TEST_ASSERT(errors.size() == 1);
        TEST_ASSERT_EQ(errors[0].message, "/root/Flag: не выполнено утверждение $value = 'true' or $value = 'false'");

        errors = support::agreedSchemaErrors(XSD_ASSERT_VALUE,
            "<root><N>12</N></root>");
        TEST_ASSERT(errors.empty());

        errors = support::agreedSchemaErrors(XSD_ASSERT_VALUE,
            "<root><N>9</N></root>");
        TEST_ASSERT(errors.size() == 1);
        TEST_ASSERT_EQ(errors[0].message, "/root/N: не выполнено утверждение $value gt 10");
    }

    void testComplexContentValue()
    {
        std::vector<beresta::Diagnostic> errors = support::agreedSchemaErrors(XSD_ASSERT_VALUE,
            "<root><Box><item>a</item></Box></root>");
        TEST_ASSERT(errors.empty());
    }
}

namespace suites
{
    // Регистрация тестов для проверки значений элементов с xs:assert в сложных типах
    // Registers the tests for checking values of elements with xs:assert in complex types
    void registerXsdAssertValueTests(test::TestRunner& runner)
    {
        runner.add("XSD 1.1: xs:assert, значения элементов", testSimpleContentValue);
        runner.add("XSD 1.1: xs:assert, сложные типы", testComplexContentValue);
    }
}
