#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Библиотека: области ключей вложены (полки внутри библиотеки), keyref
    ** библиотеки ссылается на ключ, объявленный на полке, поля — и атрибут,
    ** и элемент, селекторы с ".//" и "|". Пространство имён — с префиксом.
    **
    ** A library: key scopes are nested (shelves inside the library), the library keyref
    ** refers to a key declared on a shelf, the fields are both an attribute
    ** and an element, the selectors use ".//" and "|". The namespace has a prefix.
    */
    const char* const LIBRARY_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xmlns:l=\"urn:l\" targetNamespace=\"urn:l\""
        " elementFormDefault=\"qualified\"><xs:element name=\"lib\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"shelf\" maxOccurs=\"unbounded\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"book\" minOccurs=\"0\" maxOccurs=\"unbounded\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"isbn\" type=\"xs:decimal\" minOccurs=\"0\" maxOccurs=\"2\"/></xs:sequence>"
        "<xs:attribute name=\"id\" type=\"xs:int\"/></xs:complexType></xs:element></xs:sequence></xs:complexType>"
        "<xs:key name=\"bookId\"><xs:selector xpath=\"l:book\"/><xs:field xpath=\"@id\"/></xs:key>"
        "<xs:unique name=\"bookIsbn\"><xs:selector xpath=\"./l:book\"/><xs:field xpath=\"l:isbn\"/></xs:unique>"
        "</xs:element><xs:element name=\"ref\" minOccurs=\"0\" maxOccurs=\"unbounded\"><xs:complexType>"
        "<xs:attribute name=\"to\" type=\"xs:int\"/></xs:complexType></xs:element></xs:sequence></xs:complexType>"
        "<xs:keyref name=\"toBook\" refer=\"l:bookId\"><xs:selector xpath=\"l:ref\"/><xs:field xpath=\"@to\"/>"
        "</xs:keyref><xs:unique name=\"any\"><xs:selector xpath=\".//l:book | .//l:ref\"/>"
        "<xs:field xpath=\"@id | @to\"/></xs:unique></xs:element></xs:schema>";

    // Документ библиотеки с телом body
    // A library document with the body body
    std::string library(const std::string& body)
    {
        return "<lib xmlns=\"urn:l\">\n" + body + "</lib>";
    }

    /*
    ** Потоковые ключи SAX против ключей по дереву (XPath): повторы ключа и
    ** значения decimal (1 = 1.0), лишние и пустые поля, ссылка на ключ из
    ** области-потомка, висячая ссылка, вложенные цели «.//».
    **
    ** Streaming SAX keys against keys over the tree (XPath): repeated keys and
    ** decimal values (1 = 1.0), extra and empty fields, a reference to a key from a
    ** descendant scope, a dangling reference, nested ".//" targets.
    */
    void testStreamingKeysAgree()
    {
        const std::string bodies[] = {
            "<shelf><book id=\"1\"><isbn>1.0</isbn></book><book id=\"2\"><isbn>1</isbn></book></shelf>\n"
            "<ref to=\"2\"/><ref to=\"7\"/>\n",
            "<shelf><book id=\"1\"/><book id=\"1\"><isbn>3</isbn><isbn>4</isbn></book><book/></shelf>\n"
            "<shelf><book id=\"1\"/></shelf><ref to=\"1\"/><ref/>\n",
            "<shelf><book id=\"5\"/></shelf><shelf><book id=\"6\"/></shelf><ref to=\"6\"/><ref to=\"5\"/>\n"
        };

        for (const std::string& body : bodies)
        {
            support::SchemaRuns runs = support::runSchemaModes(LIBRARY_XSD, library(body));
            TEST_ASSERT(!runs.sax.empty());
            TEST_ASSERT_EQ(support::describeDiagnostics(runs.sax), support::describeDiagnostics(runs.parsed));
            TEST_ASSERT(support::sameDiagnostics(runs.parsed, runs.tree));
        }
    }

    /*
    ** Выражение вне грамматики XSD (здесь "a//b") при разборе в DOM
    ** вычисляет XPath по дереву, а без дерева такое ограничение не
    ** проверить — SAX сообщает об этом один раз, называя ограничение.
    **
    ** An expression outside the XSD grammar (here "a//b") is evaluated by XPath over
    ** the tree when parsing into a DOM, but without a tree such a constraint cannot be
    ** checked — SAX reports it once, naming the constraint.
    */
    void testUnstreamableConstraint()
    {
        const std::string xsd = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"><xs:element name=\"r\">"
            "<xs:complexType><xs:sequence><xs:any processContents=\"skip\" maxOccurs=\"unbounded\"/></xs:sequence>"
            "</xs:complexType><xs:unique name=\"deepItems\"><xs:selector xpath=\"g//i\"/><xs:field xpath=\"@a\"/>"
            "</xs:unique></xs:element></xs:schema>";
        support::SchemaRuns runs = support::runSchemaModes(xsd, "<r><g><i a=\"1\"/><h><i a=\"1\"/></h></g></r>");

        TEST_ASSERT(runs.parsed.size() == 1);
        TEST_ASSERT(runs.parsed.front().message.find("deepItems") != std::string::npos);
        TEST_ASSERT(runs.sax.size() == 1);
        TEST_ASSERT(runs.sax.front().message.find("\"deepItems\"") != std::string::npos);
        TEST_ASSERT(runs.sax.front().message.find("/r: ") != std::string::npos);
    }

    /*
    ** ValidatingHandler вручную: декоратор перед DomBuilder, ключи — по
    ** построенному дереву (ON_TREE). Результат — как у XmlParser со схемой.
    **
    ** ValidatingHandler by hand: the decorator before DomBuilder, the keys — over
    ** the built tree (ON_TREE). The result is the same as XmlParser with a schema gives.
    */
    void testHandlerBeforeDomBuilder()
    {
        std::string xml = library("<shelf><book id=\"1\"/><book id=\"1\"/></shelf><ref to=\"3\"/>\n");
        beresta::ParserOptions options;
        beresta::DomBuilder builder(options);
        auto schema = beresta::xsd::Schema::fromString(LIBRARY_XSD);
        beresta::xsd::ValidatingHandler validator(schema, &builder,
            beresta::xsd::validationSettingsFor(options, beresta::xsd::IdentityEvaluation::ON_TREE));

        beresta::SaxParser(options).parse(xml, validator);
        beresta::Document document = builder.takeDocument();
        validator.checkIdentity(document);
        std::vector<beresta::Diagnostic> manual = validator.takeDiagnostics();
        TEST_ASSERT(document.getRoot() != nullptr && document.getRoot()->getLocalName() == "lib");
        TEST_ASSERT(manual.size() == 3);
        TEST_ASSERT(support::sameDiagnostics(manual, support::runSchemaModes(LIBRARY_XSD, xml).parsed));
    }
}

namespace suites
{
    // Регистрация тестов потоковых ограничений идентичности
    // Registers the tests of streaming identity constraints
    void registerXsdStreamIdentityTests(test::TestRunner& runner)
    {
        runner.add("XSD поток: ключи SAX совпадают с ключами по дереву", testStreamingKeysAgree);
        runner.add("XSD поток: ограничение вне грамматики XSD", testUnstreamableConstraint);
        runner.add("XSD поток: ValidatingHandler перед DomBuilder", testHandlerBeforeDomBuilder);
    }
}
