#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    const char* const XSI = " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";

    const char* const XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:complexType name=\"Mixed\" mixed=\"true\"><xs:sequence><xs:element name=\"b\" minOccurs=\"0\"/></xs:sequence></xs:complexType>"
        "<xs:element name=\"mix\" type=\"Mixed\" fixed=\"abc\"/>"
        "<xs:element name=\"anything\" fixed=\"abc\"/>"
        "<xs:element name=\"nilFixed\" type=\"xs:string\" nillable=\"true\" fixed=\"abc\"/>"
        "<xs:element name=\"plain\" type=\"xs:string\"/>"
        "<xs:element name=\"ids\"><xs:complexType><xs:sequence><xs:element name=\"a\" maxOccurs=\"2\"><xs:complexType><xs:attribute name=\"id\" type=\"xs:ID\" default=\"x\"/></xs:complexType></xs:element></xs:sequence></xs:complexType></xs:element>"
        "<xs:element name=\"refs\"><xs:complexType><xs:attribute name=\"r\" type=\"xs:IDREFS\" default=\"nope\"/></xs:complexType></xs:element>"
        "<xs:element name=\"all\"><xs:complexType><xs:all><xs:element name=\"a1\" minOccurs=\"0\" maxOccurs=\"0\"/><xs:element name=\"a2\" minOccurs=\"0\"/></xs:all></xs:complexType></xs:element>"
        "</xs:schema>";

    /*
    ** Проверка ограничений значений по схеме: <mix>abc</mix> 0; <mix/> 0 (пустой получает fixed); <mix>xyz</mix> 1; <mix><b/></mix> 1 (детей-элементов при fixed быть не может); <anything>abc</anything> 0; <anything>q</anything> 1.
    ** Tests value constraints: <mix>abc</mix> 0; <mix/> 0 (empty gets fixed); <mix>xyz</mix> 1; <mix><b/></mix> 1 (no child elements with fixed); <anything>abc</anything> 0; <anything>q</anything> 1.
    */
    void testMixedFixed()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<mix>abc</mix>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<mix/>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<mix>xyz</mix>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<mix><b/></mix>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<anything>abc</anything>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<anything>q</anything>").size() == 1);
    }

    /*
    ** Проверка ограничений xsi:nil: <plain xsi:nil="false">t</plain> 1 (xsi:nil, даже false, у не-nillable запрещён); <nilFixed xsi:nil="true"/> 1 (nil при fixed запрещён); <nilFixed>abc</nilFixed> 0.
    ** Tests xsi:nil constraints: <plain xsi:nil="false">t</plain> 1 (xsi:nil, even false, is forbidden for non-nillable); <nilFixed xsi:nil="true"/> 1 (nil with fixed is forbidden); <nilFixed>abc</nilFixed> 0.
    */
    void testNil()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<plain") + XSI + " xsi:nil=\"false\">t</plain>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<nilFixed") + XSI + " xsi:nil=\"true\"/>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<nilFixed>abc</nilFixed>").empty());
    }

    /*
    ** Проверка ограничений по умолчанию ID: <ids><a/><a/></ids> 1 (оба получают ID x по умолчанию — повтор); <ids><a/><a id="y"/></ids> 0; <refs/> 1 (IDREFS по умолчанию ссылается на несуществующий ID).
    ** Tests default ID constraints: <ids><a/><a/></ids> 1 (both get default ID x — duplicate); <ids><a/><a id="y"/></ids> 0; <refs/> 1 (default IDREFS references non-existent ID).
    */
    void testDefaultIds()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<ids><a/><a/></ids>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<ids><a/><a id=\"y\"/></ids>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<refs/>").size() == 1);
    }

    /*
    ** Проверка ограничений maxOccurs=0 в xs:all: <all><a2/></all> 0; <all><a1/></all> 1 (элемент с maxOccurs=0 в xs:all недопустим).
    ** Tests maxOccurs=0 in xs:all: <all><a2/></all> 0; <all><a1/></all> 1 (element with maxOccurs=0 in xs:all is not allowed).
    */
    void testAllMaxOccursZero()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<all><a2/></all>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<all><a1/></all>").size() == 1);
    }
}

namespace suites
{
    // Регистрация тестов ограничений значений по схеме
    // Registers the tests of value constraints by schema
    void registerXsdValueConstraintTests(test::TestRunner& runner)
    {
        runner.add("XSD: тесты ограничений значений", testMixedFixed);
        runner.add("XSD: тесты ограничений xsi:nil", testNil);
        runner.add("XSD: тесты ограничений по умолчанию ID", testDefaultIds);
        runner.add("XSD: тесты maxOccurs=0 в xs:all", testAllMaxOccursZero);
    }
}
