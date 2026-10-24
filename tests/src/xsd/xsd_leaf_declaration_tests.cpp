#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    const char* const XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"r\"><xs:complexType><xs:sequence><xs:element name=\"v\" type=\"xs:string\" fixed=\"a\"/><xs:element name=\"v\" type=\"xs:string\" fixed=\"b\"/></xs:sequence></xs:complexType></xs:element>"
        "<xs:element name=\"w\"><xs:complexType><xs:sequence><xs:any namespace=\"urn:lax\" processContents=\"lax\"/><xs:any namespace=\"urn:strict\"/></xs:sequence></xs:complexType></xs:element>"
        "<xs:element name=\"e\" type=\"xs:int\"/>"
        "<xs:element name=\"s\"><xs:complexType><xs:sequence><xs:any processContents=\"skip\"/><xs:element ref=\"e\"/></xs:sequence></xs:complexType></xs:element>"
        "<xs:element name=\"p\"><xs:complexType><xs:attribute name=\"a\" use=\"prohibited\"/><xs:anyAttribute namespace=\"##local\" processContents=\"lax\"/></xs:complexType></xs:element>"
        "<xs:element name=\"q\"><xs:complexType><xs:attribute name=\"a\" use=\"prohibited\"/></xs:complexType></xs:element>"
        "<xs:element name=\"u1\" fixed=\"1\"><xs:simpleType><xs:union memberTypes=\"xs:boolean xs:int\"/></xs:simpleType></xs:element>"
        "<xs:element name=\"u2\" fixed=\"01\"><xs:simpleType><xs:union memberTypes=\"xs:string xs:int\"/></xs:simpleType></xs:element>"
        "<xs:element name=\"l\" fixed=\"1 2\"><xs:simpleType><xs:list itemType=\"xs:int\"/></xs:simpleType></xs:element>"
        "</xs:schema>";

    /*
    ** Проверка ограничений на элементы с одинаковыми именами: <r><v>a</v><v>b</v></r> 0; <r><v>a</v><v>a</v></r> 1 (второй v проверяется по своему fixed="b").
    ** Tests constraints on elements with the same name: <r><v>a</v><v>b</v></r> 0; <r><v>a</v><v>a</v></r> 1 (second v is checked against its fixed="b").
    */
    void testSameNameParticles()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<r><v>a</v><v>b</v></r>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<r><v>a</v><v>a</v></r>").size() == 1);
    }

    /*
    ** Проверка режимов xs:any: <w><a xmlns=\"urn:lax\"/><b xmlns=\"urn:strict\"/></w> 1 (у каждого xs:any свой processContents: ошибка только у b); <s><e>x</e><e>1</e></s> 0 (первый e принят xs:any skip и не проверяется); <s><e>x</e><e>y</e></s> 1.
    ** Tests xs:any modes: <w><a xmlns=\"urn:lax\"/><b xmlns=\"urn:strict\"/></w> 1 (each xs:any has its own processContents: error only on b); <s><e>x</e><e>1</e></s> 0 (first e is accepted by xs:any skip and not checked); <s><e>x</e><e>y</e></s> 1.
    */
    void testWildcardModes()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<w><a xmlns=\"urn:lax\"/><b xmlns=\"urn:strict\"/></w>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<s><e>x</e><e>1</e></s>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<s><e>x</e><e>y</e></s>").size() == 1);
    }

    /*
    ** Проверка запрещённых атрибутов с xs:anyAttribute: <p a=\"1\"/> 0 (запрещённый атрибут допускает anyAttribute); <q a=\"1\"/> 1.
    ** Tests prohibited attributes with xs:anyAttribute: <p a=\"1\"/> 0 (prohibited attribute allows anyAttribute); <q a=\"1\"/> 1.
    */
    void testProhibitedWithWildcard()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<p a=\"1\"/>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<q a=\"1\"/>").size() == 1);
    }

    /*
    ** Проверка ограничений на объединения и списки с фиксированными значениями: <u1>true</u1> 0 (fixed 1 и true — boolean true); <u2>01</u2> 0; <u2>1</u2> 1 (оба значения — string первого члена); <l>01  2</l> 0.
    ** Tests constraints on unions and lists with fixed values: <u1>true</u1> 0 (fixed 1 and true — boolean true); <u2>01</u2> 0; <u2>1</u2> 1 (both values — string of the first member); <l>01  2</l> 0.
    */
    void testUnionAndListFixed()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<u1>true</u1>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<u2>01</u2>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<u2>1</u2>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<l>01  2</l>").empty());
    }
}

namespace suites
{
    // Регистрация тестов ограничений на объявления XSD
    // Registers the tests of XSD declaration constraints
    void registerXsdLeafDeclarationTests(test::TestRunner& runner)
    {
        runner.add("XSD: тесты ограничений на элементы с одинаковыми именами", testSameNameParticles);
        runner.add("XSD: тесты режимов xs:any", testWildcardModes);
        runner.add("XSD: тесты запрещённых атрибутов с xs:anyAttribute", testProhibitedWithWildcard);
        runner.add("XSD: тесты ограничений на объединения и списки с фиксированными значениями", testUnionAndListFixed);
    }
}
