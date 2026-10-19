#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    const char* const XSI = " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";

    /*
    ** Схема без глобального объявления корня: тип Code (до трёх символов)
    ** и элементы-обёртки wrap (xs:any strict) и loose (xs:any lax).
    **
    ** A schema without a global declaration of the root: the type Code (up to three characters)
    ** and the wrapper elements wrap (xs:any strict) and loose (xs:any lax).
    */
    const char* const XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:simpleType name=\"Code\"><xs:restriction base=\"xs:string\"><xs:maxLength value=\"3\"/>"
        "</xs:restriction></xs:simpleType>"
        "<xs:complexType name=\"Pair\"><xs:sequence><xs:element name=\"k\" type=\"Code\"/></xs:sequence>"
        "<xs:attribute name=\"id\" type=\"xs:int\"/></xs:complexType>"
        "<xs:element name=\"wrap\"><xs:complexType><xs:sequence><xs:any processContents=\"strict\""
        " maxOccurs=\"2\"/></xs:sequence></xs:complexType></xs:element>"
        "<xs:element name=\"loose\"><xs:complexType><xs:sequence><xs:any processContents=\"lax\"/>"
        "</xs:sequence></xs:complexType></xs:element>"
        "<xs:element name=\"nil\" type=\"Code\" nillable=\"true\"/>"
        "<xs:element name=\"pairs\" type=\"Pair\" nillable=\"true\"/></xs:schema>";

    /*
    ** Корень без объявления, но с xsi:type проверяется по этому типу
    ** (XSD 1.0, 3.3.4, п. 1.2; случаи ctZ011, ST_targetNS…, addB187):
    ** "ab" подходит Code, "abcd" — нет; неизвестный тип — ошибка xsi:type,
    ** а без xsi:type корень по-прежнему «не объявлен».
    **
    ** A root without a declaration but with xsi:type is validated against that type
    ** (XSD 1.0, 3.3.4, item 1.2; the cases ctZ011, ST_targetNS…, addB187):
    ** "ab" fits Code, "abcd" does not; an unknown type is an xsi:type error,
    ** and without xsi:type the root is still "undeclared".
    */
    void testUndeclaredRootWithXsiType()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<r") + XSI + " xsi:type=\"Code\">ab</r>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<r") + XSI
            + " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xsi:type=\"xs:anyType\"><x/></r>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<r") + XSI + " xsi:type=\"Code\">abcd</r>")
            .size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<r") + XSI + " xsi:type=\"Nope\">ab</r>")
            .size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<r>ab</r>").size() == 1);
    }

    /*
    ** Ребёнок xs:any без глобального объявления: с xsi:type — проверка по
    ** типу и при strict, и при lax (addB116, particlesB013); без xsi:type
    ** strict — ошибка, lax — не проверяется.
    **
    ** An xs:any child without a global declaration: with xsi:type — validation by
    ** the type both with strict and with lax (addB116, particlesB013); without xsi:type
    ** strict is an error, lax is not validated.
    */
    void testWildcardChildWithXsiType()
    {
        std::string open = std::string("<wrap") + XSI + ">";

        TEST_ASSERT(support::agreedSchemaErrors(XSD, open + "<a xsi:type=\"Code\">ab</a>"
            "<b xsi:type=\"Pair\" id=\"1\"><k>x</k></b></wrap>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, open + "<a xsi:type=\"Code\">abcd</a>"
            "<b xsi:type=\"Pair\" id=\"z\"><k>x</k></b></wrap>").size() == 2);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, open + "<a>ab</a><b>?</b></wrap>").size() == 2);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<loose") + XSI + "><a xsi:type=\"Code\">abcd</a>"
            "</loose>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<loose><a>?</a></loose>").empty());
    }

    /*
    ** Значения xsi:type и xsi:nil сворачиваются (whiteSpace="collapse"):
    ** пробелы по краям не мешают (sunData/ElemDecl typeDef00601m).
    **
    ** The values of xsi:type and xsi:nil are collapsed (whiteSpace="collapse"):
    ** the spaces at the edges do not interfere (sunData/ElemDecl typeDef00601m).
    */
    void testXsiValueSpaces()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<r") + XSI
            + " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xsi:type=\"   xs:boolean \">true</r>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<nil") + XSI + " xsi:nil=\" true \"/>").empty());
    }

    /*
    ** xsi:nil="true" отменяет проверку содержимого, но не атрибутов
    ** (sunData/ElemDecl typeDef01201m1_n): у простого типа посторонний
    ** атрибут — ошибка, у составного атрибуты проверяются по типу.
    **
    ** xsi:nil="true" cancels the content check, but not the attributes one
    ** (sunData/ElemDecl typeDef01201m1_n): for a simple type a foreign
    ** attribute is an error, for a complex one the attributes are validated by the type.
    */
    void testNilKeepsAttributeChecks()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<nil") + XSI + " xsi:nil=\"true\" a=\"1\"/>")
            .size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<pairs") + XSI + " xsi:nil=\"true\" id=\"z\"/>")
            .size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, std::string("<pairs") + XSI + " xsi:nil=\"true\" id=\"7\"/>")
            .empty());
    }
}

namespace suites
{
    // Регистрация тестов xsi:type без объявления элемента и xsi:nil с атрибутами
    // Registers the tests of xsi:type without an element declaration and of xsi:nil with attributes
    void registerXsdXsiTypeRootTests(test::TestRunner& runner)
    {
        runner.add("XSD: корень без объявления с xsi:type", testUndeclaredRootWithXsiType);
        runner.add("XSD: ребёнок xs:any с xsi:type", testWildcardChildWithXsiType);
        runner.add("XSD: пробелы в xsi:type и xsi:nil", testXsiValueSpaces);
        runner.add("XSD: xsi:nil не отменяет проверку атрибутов", testNilKeepsAttributeChecks);
    }
}
