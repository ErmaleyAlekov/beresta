#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Схема с пространством имён, наследованием, simpleContent, xs:all, xs:any, группами и ID
    const char* const ADVANCED_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xmlns:t=\"urn:t\""
        " targetNamespace=\"urn:t\" elementFormDefault=\"qualified\">"
        "<xs:complexType name=\"Base\"><xs:sequence><xs:element name=\"id\" type=\"xs:ID\"/>"
        "</xs:sequence></xs:complexType>"
        "<xs:complexType name=\"Item\"><xs:complexContent><xs:extension base=\"t:Base\"><xs:sequence>"
        "<xs:element name=\"price\" type=\"t:Price\"/><xs:element name=\"ref\" type=\"xs:IDREF\" minOccurs=\"0\"/>"
        "<xs:group ref=\"t:Extra\" minOccurs=\"0\"/></xs:sequence>"
        "<xs:attributeGroup ref=\"t:Common\"/></xs:extension></xs:complexContent></xs:complexType>"
        "<xs:complexType name=\"Price\"><xs:simpleContent><xs:extension base=\"xs:decimal\">"
        "<xs:attribute name=\"currency\" type=\"xs:string\" use=\"required\"/></xs:extension>"
        "</xs:simpleContent></xs:complexType>"
        "<xs:group name=\"Extra\"><xs:all><xs:element name=\"a\" type=\"xs:int\"/>"
        "<xs:element name=\"b\" type=\"xs:int\" minOccurs=\"0\"/></xs:all></xs:group>"
        "<xs:attributeGroup name=\"Common\"><xs:attribute name=\"note\" type=\"xs:string\"/></xs:attributeGroup>"
        "<xs:element name=\"shop\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"item\" type=\"t:Item\" maxOccurs=\"unbounded\" nillable=\"true\"/>"
        "<xs:any namespace=\"##other\" processContents=\"skip\" minOccurs=\"0\"/>"
        "</xs:sequence></xs:complexType></xs:element></xs:schema>";

    // Число ошибок для документа в пространстве urn:t
    std::size_t errorsFor(const std::string& body)
    {
        static const std::shared_ptr<const beresta::xsd::Schema> SCHEMA =
            beresta::xsd::Schema::fromString(ADVANCED_XSD);
        beresta::Document document = beresta::XmlParser().parse(
            "<shop xmlns=\"urn:t\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">" + body + "</shop>");

        return beresta::xsd::SchemaValidator(SCHEMA).validate(document).size();
    }

    /*
    ** Корректный документ: расширение типа, simpleContent, xs:all в любом
    ** порядке, xs:any, ID/IDREF, xsi:nil. Затем по одному нарушению
    ** каждого правила: атрибут simpleContent, xs:all, IDREF, повтор ID, nil, ##other.
    */
    void testAdvancedRules()
    {
        TEST_ASSERT(errorsFor("<item note=\"n\"><id>i1</id><price currency=\"RUB\">10.5</price></item>"
            "<item><id>i2</id><price currency=\"USD\">1</price><ref>i1</ref><b>2</b><a>1</a></item>"
            "<item xsi:nil=\"true\"/><x:any xmlns:x=\"urn:other\"><free/></x:any>") == 0);
        TEST_ASSERT(errorsFor("<item><id>i1</id><price>10</price></item>") == 1);
        TEST_ASSERT(errorsFor("<item><id>i1</id><price currency=\"R\">1</price><b>1</b></item>") == 1);
        TEST_ASSERT(errorsFor("<item><id>i1</id><price currency=\"R\">1</price><ref>none</ref></item>") == 1);
        TEST_ASSERT(errorsFor("<item><id>i1</id><price currency=\"R\">1</price></item>"
            "<item><id>i1</id><price currency=\"R\">2</price></item>") == 1);
        TEST_ASSERT(errorsFor("<item xsi:nil=\"true\"><id>x</id></item>") == 1);
        TEST_ASSERT(errorsFor("<item><id>i1</id><price currency=\"R\">1</price></item><other/>") == 1);
    }

    // Проверка схемой прямо при разборе: ошибки со строками приходят в ErrorHandler
    void testParserIntegration()
    {
        beresta::CollectingErrorHandler handler;
        beresta::ParserOptions options;
        options.schema = beresta::xsd::Schema::fromString(ADVANCED_XSD);
        options.errorHandler = &handler;

        beresta::XmlParser(options).parse("<shop xmlns=\"urn:t\">\n<item>\n<id>1x</id>\n"
            "<price currency=\"R\">abc</price>\n</item>\n</shop>");
        TEST_ASSERT(handler.getDiagnostics().size() == 2);
        TEST_ASSERT(handler.getDiagnostics()[0].position.line == 3);
        beresta::ParserOptions strict;
        strict.schema = options.schema;
        TEST_ASSERT_THROWS(beresta::XmlParser(strict).parse("<shop xmlns=\"urn:t\"/>"), beresta::ParseException);
    }

    // xs:include через FileResourceResolver
    void testSchemaInclude()
    {
        std::filesystem::path root = support::makeTestDirectory("xsd_include");
        support::writeTestFile(root / "types.xsd", "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
            "<xs:simpleType name=\"Code\"><xs:restriction base=\"xs:string\"><xs:length value=\"3\"/>"
            "</xs:restriction></xs:simpleType></xs:schema>");
        support::writeTestFile(root / "main.xsd", "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
            "<xs:include schemaLocation=\"types.xsd\"/><xs:element name=\"code\" type=\"Code\"/></xs:schema>");
        beresta::FileResourceResolver resolver(root);
        beresta::xsd::SchemaLoadOptions options;
        options.resolver = &resolver;

        auto schema = beresta::xsd::Schema::fromFile(root / "main.xsd", options);
        beresta::xsd::SchemaValidator validator(schema);
        TEST_ASSERT(validator.isValid(beresta::XmlParser().parse("<code>ABC</code>")));
        TEST_ASSERT(!validator.isValid(beresta::XmlParser().parse("<code>ABCD</code>")));
    }
}

namespace suites
{
    // Регистрация расширенных тестов XSD
    void registerXsdAdvancedTests(test::TestRunner& runner)
    {
        runner.add("XSD: наследование, all, any, ID и их нарушения", testAdvancedRules);
        runner.add("XSD: проверка при разборе", testParserIntegration);
        runner.add("XSD: xs:include", testSchemaInclude);
    }
}
