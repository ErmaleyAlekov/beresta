#include "suites/test_suites.hpp"
#include "beresta.hpp"

#include <string>

namespace
{
    /*
    ** final типов — ошибки самой схемы: расширение и ограничение составного
    ** типа, ограничение, список и объединение простого, finalDefault="#all".
    **
    ** The final of types is an error of the schema itself: an extension and a restriction
    ** of a complex type, a restriction, a list and a union of a simple one, finalDefault="#all".
    */
    void testFinalPreventsDerivation()
    {
        std::string head = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"";
        std::string base = "><xs:complexType name=\"B\" final=\"extension\"/>";
        std::string simple = "><xs:simpleType name=\"S\" final=\"restriction list union\">"
            "<xs:restriction base=\"xs:string\"/></xs:simpleType>";

        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(head + base + "<xs:complexType name=\"D\">"
            "<xs:complexContent><xs:extension base=\"B\"/></xs:complexContent></xs:complexType></xs:schema>"),
            beresta::DomException);
        TEST_ASSERT(beresta::xsd::Schema::fromString(head + base + "<xs:complexType name=\"D\">"
            "<xs:complexContent><xs:restriction base=\"B\"/></xs:complexContent></xs:complexType></xs:schema>")
            != nullptr);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(head + simple + "<xs:simpleType name=\"D\">"
            "<xs:restriction base=\"S\"/></xs:simpleType></xs:schema>"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(head + simple + "<xs:simpleType name=\"D\">"
            "<xs:list itemType=\"S\"/></xs:simpleType></xs:schema>"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(head + simple + "<xs:simpleType name=\"D\">"
            "<xs:union memberTypes=\"S xs:int\"/></xs:simpleType></xs:schema>"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(head + " finalDefault=\"#all\">"
            "<xs:complexType name=\"B\"/><xs:complexType name=\"D\"><xs:complexContent>"
            "<xs:restriction base=\"B\"/></xs:complexContent></xs:complexType></xs:schema>"),
            beresta::DomException);
    }

    /*
    ** Без запретов те же цепочки наследования загружаются: значения по
    ** умолчанию finalDefault / blockDefault не мешают, если они пусты, а
    ** block на элементе — запрет для документа, а не для схемы.
    **
    ** Without prohibitions the same derivation chains load: the finalDefault /
    ** blockDefault defaults do not interfere when they are empty, and
    ** block on an element is a prohibition for the document, not for the schema.
    */
    void testDerivationWithoutFinal()
    {
        TEST_ASSERT(beresta::xsd::Schema::fromString("<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
            " blockDefault=\"#all\"><xs:complexType name=\"B\"/><xs:complexType name=\"D\"><xs:complexContent>"
            "<xs:extension base=\"B\"/></xs:complexContent></xs:complexType><xs:simpleType name=\"S\">"
            "<xs:restriction base=\"xs:int\"/></xs:simpleType><xs:simpleType name=\"L\"><xs:list itemType=\"S\"/>"
            "</xs:simpleType><xs:element name=\"e\" type=\"B\" block=\"#all\"/></xs:schema>") != nullptr);
    }
}

namespace suites
{
    // Регистрация тестов final: запреты наследования проверяются при загрузке схемы
    // Registers the final tests: derivation prohibitions are checked when the schema is loaded
    void registerXsdFinalTests(test::TestRunner& runner)
    {
        runner.add("XSD: final запрещает наследование", testFinalPreventsDerivation);
        runner.add("XSD: наследование без запретов final", testDerivationWithoutFinal);
    }
}
