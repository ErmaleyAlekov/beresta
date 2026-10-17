#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "support/parse_helpers.hpp"

namespace
{
    // Схема каталога: последовательность, атрибуты, перечисление, шаблон, повторения
    const char* const CATALOG_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:simpleType name=\"Isbn\"><xs:restriction base=\"xs:string\">"
        "<xs:pattern value=\"\\d{3}-\\d{10}\"/></xs:restriction></xs:simpleType>"
        "<xs:simpleType name=\"Lang\"><xs:restriction base=\"xs:token\">"
        "<xs:enumeration value=\"ru\"/><xs:enumeration value=\"en\"/></xs:restriction></xs:simpleType>"
        "<xs:element name=\"catalog\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"book\" maxOccurs=\"unbounded\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"title\" type=\"xs:string\"/>"
        "<xs:element name=\"price\"><xs:simpleType><xs:restriction base=\"xs:decimal\">"
        "<xs:minExclusive value=\"0\"/><xs:fractionDigits value=\"2\"/></xs:restriction></xs:simpleType></xs:element>"
        "<xs:element name=\"tag\" type=\"xs:NCName\" minOccurs=\"0\" maxOccurs=\"3\"/>"
        "</xs:sequence><xs:attribute name=\"isbn\" type=\"Isbn\" use=\"required\"/>"
        "<xs:attribute name=\"lang\" type=\"Lang\" default=\"ru\"/></xs:complexType></xs:element>"
        "</xs:sequence></xs:complexType></xs:element></xs:schema>";

    // Корректный документ — ни одной ошибки
    void testValidCatalog()
    {
        std::vector<beresta::Diagnostic> errors = support::schemaErrors(CATALOG_XSD, 
            "<catalog><book isbn=\"978-5170908780\" lang=\"en\"><title>Dune</title><price>9.99</price>"
            "<tag>фантастика</tag></book><book isbn=\"978-0000000000\"><title>T</title><price>1</price>"
            "</book></catalog>");

        for (const beresta::Diagnostic& error : errors)
            TEST_ASSERT_EQ(error.message, "");
        TEST_ASSERT(errors.empty());
    }

    // Все нарушения собираются за один проход, у каждой ошибки — путь к элементу
    void testCollectsSchemaErrors()
    {
        std::vector<beresta::Diagnostic> errors = support::schemaErrors(CATALOG_XSD, 
            "<catalog><book isbn=\"bad\" lang=\"de\"><title>A</title><price>-1</price></book>"
            "<book><price>1.234</price><title>B</title></book><book isbn=\"978-0000000000\" extra=\"1\">"
            "<title>C</title><price>2</price><tag>1</tag></book></catalog>");

        TEST_ASSERT(errors.size() >= 7);
        TEST_ASSERT(errors[0].message.find("/catalog/book[1]") == 0);
    }

    // Модель содержимого: порядок, обязательность, лишние элементы, текст в element-only
    void testContentModel()
    {
        TEST_ASSERT(support::schemaErrors(CATALOG_XSD, "<catalog/>").size() == 1);
        TEST_ASSERT(support::schemaErrors(CATALOG_XSD, "<catalog>текст<book isbn=\"978-0000000000\"><title>A</title>"
            "<price>1</price></book></catalog>").size() == 1);
        TEST_ASSERT(support::schemaErrors(CATALOG_XSD, "<other/>").size() == 1);
        TEST_ASSERT(support::schemaErrors(CATALOG_XSD, "<catalog><book isbn=\"978-0000000000\"><title>A</title><price>1</price>"
            "<tag>a</tag><tag>b</tag><tag>c</tag><tag>d</tag></book></catalog>").size() == 1);
    }

    // Ошибки самой схемы: неизвестный тип, неверный шаблон, неизвестный фасет
    void testSchemaErrors()
    {
        const char* head = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">";

        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(std::string(head)
            + "<xs:element name=\"a\" type=\"Missing\"/></xs:schema>"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(std::string(head)
            + "<xs:simpleType name=\"t\"><xs:restriction base=\"xs:string\"><xs:pattern value=\"(\"/>"
            "</xs:restriction></xs:simpleType></xs:schema>"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString("<schema/>"), beresta::DomException);
    }
}

namespace suites
{
    // Регистрация базовых тестов проверки по схеме
    void registerXsdSchemaTests(test::TestRunner& runner)
    {
        runner.add("XSD: корректный документ", testValidCatalog);
        runner.add("XSD: сбор ошибок с путями", testCollectsSchemaErrors);
        runner.add("XSD: модель содержимого", testContentModel);
        runner.add("XSD: ошибки схемы", testSchemaErrors);
    }
}
