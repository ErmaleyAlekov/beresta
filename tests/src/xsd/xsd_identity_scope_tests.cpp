#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "support/parse_helpers.hpp"

namespace
{
    // keyref в области-предке находит ключ, объявленный на элементе-потомке
    // A keyref in an ancestor scope finds a key declared on a descendant element
    void testKeyFromDescendantScope()
    {
        std::string xsd = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"><xs:element name=\"lib\">"
            "<xs:complexType><xs:sequence><xs:element name=\"books\"><xs:complexType><xs:sequence>"
            "<xs:element name=\"book\" maxOccurs=\"unbounded\"><xs:complexType><xs:attribute name=\"id\"/>"
            "</xs:complexType></xs:element></xs:sequence></xs:complexType><xs:key name=\"bk\">"
            "<xs:selector xpath=\"book\"/><xs:field xpath=\"@id\"/></xs:key></xs:element>"
            "<xs:element name=\"ref\" maxOccurs=\"unbounded\"><xs:complexType><xs:attribute name=\"to\"/>"
            "</xs:complexType></xs:element></xs:sequence></xs:complexType><xs:keyref name=\"r\" refer=\"bk\">"
            "<xs:selector xpath=\"ref\"/><xs:field xpath=\"@to\"/></xs:keyref></xs:element></xs:schema>";
        std::string books = "<lib><books><book id=\"a\"/><book id=\"b\"/></books>";

        TEST_ASSERT(support::schemaErrors(xsd, books + "<ref to=\"b\"/></lib>").empty());
        TEST_ASSERT(support::schemaErrors(xsd, books + "<ref to=\"c\"/></lib>").size() == 1);
    }

    // Ошибки схемы: ключ refer не объявлен, разное число полей, повтор имени, неверный xpath
    // Schema errors: the refer key is not declared, a different number of fields, a repeated name, an invalid xpath
    void testIdentitySchemaErrors()
    {
        std::string head = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"><xs:element name=\"r\">";
        std::string key = "<xs:key name=\"k\"><xs:selector xpath=\"i\"/><xs:field xpath=\"@a\"/></xs:key>";
        std::string tail = "</xs:element></xs:schema>";

        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(head + "<xs:keyref name=\"x\" refer=\"none\">"
            "<xs:selector xpath=\"i\"/><xs:field xpath=\"@a\"/></xs:keyref>" + tail), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(head + key + "<xs:keyref name=\"x\" refer=\"k\">"
            "<xs:selector xpath=\"i\"/><xs:field xpath=\"@a\"/><xs:field xpath=\"@b\"/></xs:keyref>" + tail),
            beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(head + key + key + tail), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(head + "<xs:unique name=\"u\"><xs:selector xpath=\"[\"/>"
            "<xs:field xpath=\"@a\"/></xs:unique>" + tail), beresta::DomException);
    }
}

namespace suites
{
    // Регистрация тестов областей видимости ключей и ошибок ограничений в схеме
    // Registers the tests of key scopes and constraint errors in the schema
    void registerXsdIdentityScopeTests(test::TestRunner& runner)
    {
        runner.add("XSD: keyref к ключу из области-потомка", testKeyFromDescendantScope);
        runner.add("XSD: ошибки ограничений идентичности в схеме", testIdentitySchemaErrors);
    }
}
