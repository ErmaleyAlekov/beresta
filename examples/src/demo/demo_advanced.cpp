#include "demo/demo_scenarios.hpp"
#include "demo/element_counter.hpp"
#include "beresta.hpp"

#include <iostream>

namespace demo
{
    // Магазин для демонстраций XPath, схемы и SAX
    // A shop for the XPath, schema and SAX demos
    static const char* const SHOP =
        "<shop xmlns=\"urn:shop\"><item code=\"A1\"><name>Чай</name><price>120.50</price></item>"
        "<item code=\"B2\"><name>Кофе</name><price>340</price></item>"
        "<item code=\"C3\"><name>Сахар</name><price>-5</price></item></shop>";

    // XPath: префикс в выражении связывается с пространством имён документа
    // XPath: a prefix in the expression is bound to a namespace of the document
    void showXPath()
    {
        beresta::Document document = beresta::XmlParser().parse(SHOP);
        beresta::xpath::XPathContext context;
        context.namespaces.emplace("s", "urn:shop");

        beresta::xpath::XPathExpression expensive("//s:item[s:price > 200]/s:name");
        beresta::xpath::XPathExpression total("sum(//s:price)");
        std::cout << "== 5. XPath ==\n"
                  << "дороже 200: " << expensive.evaluate(document, context).toString() << '\n'
                  << "сумма цен: " << total.evaluate(document, context).toString() << "\n\n";
    }

    // XML Schema: все ошибки сразу, с путём к элементу и номером строки
    // XML Schema: all errors at once, with the element path and the line number
    void showSchemaValidation()
    {
        const char* xsd = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" targetNamespace=\"urn:shop\""
            " elementFormDefault=\"qualified\"><xs:element name=\"shop\"><xs:complexType><xs:sequence>"
            "<xs:element name=\"item\" maxOccurs=\"unbounded\"><xs:complexType><xs:sequence>"
            "<xs:element name=\"name\" type=\"xs:string\"/><xs:element name=\"price\"><xs:simpleType>"
            "<xs:restriction base=\"xs:decimal\"><xs:minInclusive value=\"0\"/></xs:restriction>"
            "</xs:simpleType></xs:element></xs:sequence><xs:attribute name=\"code\" use=\"required\">"
            "<xs:simpleType><xs:restriction base=\"xs:string\"><xs:pattern value=\"[A-Z]\\d\"/>"
            "</xs:restriction></xs:simpleType></xs:attribute></xs:complexType></xs:element>"
            "</xs:sequence></xs:complexType></xs:element></xs:schema>";
        beresta::CollectingErrorHandler handler;
        beresta::ParserOptions options;
        options.schema = beresta::xsd::Schema::fromString(xsd);
        options.errorHandler = &handler;

        beresta::XmlParser(options).parse(SHOP);
        std::cout << "== 6. XML Schema ==\n";
        for (const beresta::Diagnostic& diagnostic : handler.getDiagnostics())
            std::cout << diagnostic.message << '\n';
        std::cout << '\n';
    }

    // SAX: обработчик получает события, дерево не строится
    // SAX: the handler receives events, no tree is built
    void showSaxStreaming()
    {
        ElementCounter counter;

        beresta::SaxParser().parse(SHOP, counter);
        std::cout << "== 7. SAX ==\n" << "элементов: " << counter.getElementCount()
                  << ", сумма цен: " << counter.getTotalPrice() << "\n\n";
    }

    // windows-1251 на входе (определяется по декларации), красивый UTF-8 на выходе
    // windows-1251 on input (detected from the declaration), pretty UTF-8 on output
    void showEncodings()
    {
        std::string cp1251 = "<?xml version=\"1.0\" encoding=\"windows-1251\"?><a><b>\xCF\xF0\xE8\xE2\xE5\xF2</b></a>";
        beresta::Document document = beresta::XmlParser().parse(cp1251);
        beresta::SerializerOptions options;
        options.prettyPrint = true;
        options.indent = "  ";

        std::cout << "== 8. Кодировки и форматированный вывод ==\n"
                  << beresta::XmlSerializer(options).toString(document);
    }
}
