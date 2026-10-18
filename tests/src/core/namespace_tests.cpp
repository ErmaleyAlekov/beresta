#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

#include <memory>

namespace
{
    // Документ SOAP: пространство по умолчанию и префиксное
    // A SOAP document: the default and a prefixed namespace
    const char* const SOAP =
        "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
        " xmlns=\"urn:default\"><soap:Body><item a:id=\"7\" xmlns:a=\"urn:a\"/></soap:Body>"
        "</soap:Envelope>";

    // URI элементов вычисляются по объявлениям в предках
    // The URIs of elements are computed from the declarations in the ancestors
    void testNamespaceResolution()
    {
        beresta::Document document = beresta::XmlParser().parse(SOAP);
        const beresta::Element* root = document.getRoot();
        const beresta::Element* body = root->findChildElementNS(
            "http://schemas.xmlsoap.org/soap/envelope/", "Body");

        TEST_ASSERT(body != nullptr);
        TEST_ASSERT_EQ(std::string(body->getLocalName()), "Body");
        TEST_ASSERT_EQ(std::string(body->getPrefix()), "soap");
        const beresta::Element* item = body->findChildElement("item");
        TEST_ASSERT_EQ(item->getNamespaceURI().value(), "urn:default");
        TEST_ASSERT_EQ(item->getAttributeNS("urn:a", "id").value(), "7");
        TEST_ASSERT_EQ(item->lookupPrefix("http://schemas.xmlsoap.org/soap/envelope/").value(), "soap");
        TEST_ASSERT(document.getElementsByTagNameNS("urn:default", "*").size() == 1);
    }

    // setAttributeNS сам объявляет префикс, если он ещё не связан
    // setAttributeNS declares the prefix itself if it is not bound yet
    void testSetAttributeNamespace()
    {
        beresta::Element element("e");

        element.setAttributeNS("urn:x", "x:flag", "1");
        TEST_ASSERT_EQ(element.toString(), "<e xmlns:x=\"urn:x\" x:flag=\"1\"/>");
        element.setAttributeNS("urn:x", "x:flag", "2");
        TEST_ASSERT_EQ(element.getAttributeNS("urn:x", "flag").value(), "2");
        TEST_ASSERT(element.removeAttributeNS("urn:x", "flag"));
        TEST_ASSERT_THROWS(element.setAttributeNS("urn:x", "bad", "v"), beresta::DomException);
        TEST_ASSERT_THROWS(element.declareNamespace("xmlns", "urn:y"), beresta::DomException);
    }

    // Нарушения правил пространств имён при разборе — ошибки с кодом NAMESPACE_ERROR
    // Namespace rule violations while parsing are errors with the NAMESPACE_ERROR code
    void testNamespaceParseErrors()
    {
        beresta::XmlParser parser;
        const char* inputs[] = {
            "<p:a/>", "<a xmlns:p=\"urn:1\" xmlns:q=\"urn:1\" p:x=\"1\" q:x=\"2\"/>",
            "<a xmlns:p=\"\"/>", "<a xmlns:xml=\"urn:other\"/>", "<a b:c:d=\"1\"/>"
        };

        for (const char* input : inputs)
        {
            beresta::DomErrorCode code = beresta::DomErrorCode::PARSE_ERROR;
            try
            {
                parser.parse(input);
            }
            catch (const beresta::ParseException& error)
            {
                code = error.getCode();
            }
            TEST_ASSERT(code == beresta::DomErrorCode::NAMESPACE_ERROR);
        }
    }

    // Без поддержки пространств имён префиксы — просто часть имени
    // Without namespace support prefixes are just part of the name
    void testNamespaceUnaware()
    {
        beresta::ParserOptions options;
        options.namespaceAware = false;

        TEST_ASSERT_EQ(beresta::XmlParser(options).parse("<p:a q:b=\"1\"/>").toString(),
            "<p:a q:b=\"1\"/>");
    }
}

namespace suites
{
    // Регистрация тестов пространств имён
    // Registers the namespace tests
    void registerNamespaceTests(test::TestRunner& runner)
    {
        runner.add("пространства имён: разрешение URI", testNamespaceResolution);
        runner.add("пространства имён: setAttributeNS", testSetAttributeNamespace);
        runner.add("пространства имён: ошибки разбора", testNamespaceParseErrors);
        runner.add("пространства имён: режим без поддержки", testNamespaceUnaware);
    }
}
