#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    const char* const NS = " xmlns:t=\"urn:t\" xmlns:o=\"urn:o\" xmlns:p=\"urn:p\"";

    const char* const XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xmlns:t=\"urn:t\" targetNamespace=\"urn:t\">"
        "<xs:attribute name=\"num\" type=\"xs:integer\"/>"
        "<xs:attribute name=\"id1\" type=\"xs:ID\"/>"
        "<xs:attribute name=\"id2\" type=\"xs:ID\"/>"
        "<xs:element name=\"n\" type=\"xs:int\"/>"
        "<xs:element name=\"other\">"
        "<xs:complexType>"
        "<xs:anyAttribute namespace=\"##other\" processContents=\"skip\"/>"
        "</xs:complexType>"
        "</xs:element>"
        "<xs:element name=\"local\">"
        "<xs:complexType>"
        "<xs:anyAttribute namespace=\"##local\" processContents=\"skip\"/>"
        "</xs:complexType>"
        "</xs:element>"
        "<xs:element name=\"strict\">"
        "<xs:complexType>"
        "<xs:anyAttribute/>"
        "</xs:complexType>"
        "</xs:element>"
        "<xs:element name=\"lax\">"
        "<xs:complexType>"
        "<xs:anyAttribute processContents=\"lax\"/>"
        "</xs:complexType>"
        "</xs:element>"
        "<xs:complexType name=\"Base\">"
        "<xs:anyAttribute namespace=\"urn:o\" processContents=\"skip\"/>"
        "</xs:complexType>"
        "<xs:complexType name=\"Ext\">"
        "<xs:complexContent>"
        "<xs:extension base=\"t:Base\">"
        "<xs:anyAttribute namespace=\"##local\" processContents=\"skip\"/>"
        "</xs:extension>"
        "</xs:complexContent>"
        "</xs:complexType>"
        "<xs:complexType name=\"Res\">"
        "<xs:complexContent>"
        "<xs:restriction base=\"t:Base\"/>"
        "</xs:complexContent>"
        "</xs:complexType>"
        "<xs:element name=\"ext\" type=\"t:Ext\"/>"
        "<xs:element name=\"res\" type=\"t:Res\"/>"
        "<xs:attributeGroup name=\"G\">"
        "<xs:anyAttribute namespace=\"urn:o urn:p\" processContents=\"skip\"/>"
        "</xs:attributeGroup>"
        "<xs:element name=\"both\">"
        "<xs:complexType>"
        "<xs:attributeGroup ref=\"t:G\"/>"
        "<xs:anyAttribute namespace=\"urn:o ##local\" processContents=\"skip\"/>"
        "</xs:complexType>"
        "</xs:element>"
        "<xs:element name=\"any\"/>"
        "</xs:schema>";

    /*
    ** Проверяет ограничение пространства имён для xs:anyAttribute.
    ** Проверяет случаи, когда атрибуты находятся в разных пространствах имён.
    **
    ** Tests the namespace constraint for xs:anyAttribute.
    ** Tests cases where attributes are in different namespaces.
    */
    void testNamespaceConstraint()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:other") + NS + " o:a=\"1\"/>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:other") + NS + " t:a=\"1\"/>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:other") + NS + " a=\"1\"/>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:local") + NS + " a=\"1\"/>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:local") + NS + " o:a=\"1\"/>").size() == 1);
    }

    /*
    ** Проверяет атрибут processContents для xs:anyAttribute.
    ** Проверяет случаи, когда атрибуты строго или лояльно обрабатываются.
    **
    ** Tests the processContents attribute for xs:anyAttribute.
    ** Tests cases where attributes are strictly or laxly processed.
    */
    void testProcessContents()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:strict") + NS + " t:num=\"5\"/>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:strict") + NS + " t:num=\"x\"/>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:strict") + NS + " o:a=\"1\"/>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:lax") + NS + " t:num=\"x\"/>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:lax") + NS + " o:a=\"1\"/>").empty());
    }

    /*
    ** Проверяет наследование и расширение шаблонов xs:anyAttribute.
    ** Проверяет случаи, когда шаблоны объединяются или ограничиваются.
    **
    ** Tests the inheritance and extension of xs:anyAttribute templates.
    ** Tests cases where templates are combined or restricted.
    */
    void testDerivedWildcards()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:ext") + NS + " o:a=\"1\" a=\"2\"/>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:ext") + NS + " t:a=\"1\"/>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:res") + NS + " o:a=\"1\"/>").size() == 1);
    }

    /*
    ** Проверяет пересечение групп атрибутов xs:anyAttribute.
    ** Проверяет случаи, когда атрибуты находятся в разных группах.
    **
    ** Tests the intersection of attribute groups for xs:anyAttribute.
    ** Tests cases where attributes are in different groups.
    */
    void testGroupIntersection()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:both") + NS + " o:a=\"1\"/>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:both") + NS + " p:a=\"1\"/>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:both") + NS + " a=\"1\"/>").size() == 1);
    }

    /*
    ** cvc-complex-type.5: из xs:anyAttribute допустим не больше одного атрибута
    ** типа ID, даже с разными значениями.
    **
    ** cvc-complex-type.5: at most one attribute of the ID type is allowed from
    ** xs:anyAttribute, even with different values.
    */
    void testWildcardIds()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:lax") + NS + " t:id1=\"a\" t:id2=\"b\"/>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:lax") + NS + " t:id1=\"a\"/>").empty());
    }

    /*
    ** Элемент без типа (xs:anyType) проверяет детей и атрибуты как lax:
    ** по глобальному объявлению, если оно есть, иначе не проверяет.
    **
    ** An element without a type (xs:anyType) validates children and attributes as lax:
    ** by the global declaration if there is one, otherwise it does not validate them.
    */
    void testAnyTypeLax()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:any") + NS + "><t:n>x</t:n></t:any>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:any") + NS + "><t:zzz>x</t:zzz></t:any>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<t:any") + NS + " t:num=\"x\"/>").size() == 1);
    }
}

namespace suites
{
    // Регистрация тестов для xs:anyAttribute
    // Registers the tests for xs:anyAttribute
    void registerXsdAttributeWildcardTests(test::TestRunner& runner)
    {
        runner.add("XSD: ограничение пространства имён для xs:anyAttribute", testNamespaceConstraint);
        runner.add("XSD: атрибут processContents для xs:anyAttribute", testProcessContents);
        runner.add("XSD: наследование и расширение шаблонов xs:anyAttribute", testDerivedWildcards);
        runner.add("XSD: пересечение групп атрибутов xs:anyAttribute", testGroupIntersection);
        runner.add("XSD: уникальность атрибутов типа ID в xs:anyAttribute", testWildcardIds);
        runner.add("XSD: обработка xs:any с processContents=\"lax\" и типом anyType", testAnyTypeLax);
    }
}
