#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Каталог для всех тестов XPath
    const char* const CATALOG =
        "<catalog><book id=\"1\" lang=\"ru\"><title>Война и мир</title><price>300</price></book>"
        "<book id=\"2\" lang=\"en\"><title>Dune</title><price>250</price></book>"
        "<book id=\"3\" lang=\"ru\"><title>Мастер и Маргарита</title><price>400</price></book>"
        "<!-- конец --></catalog>";

    // Значение выражения строкой
    std::string eval(const beresta::Node& context, const char* expression)
    {
        return beresta::xpath::evaluateString(context, expression);
    }

    // Абсолютные и относительные пути, // и предикаты
    void testLocationPaths()
    {
        beresta::Document document = beresta::XmlParser().parse(CATALOG);

        TEST_ASSERT(beresta::xpath::selectElements(document, "/catalog/book").size() == 3);
        TEST_ASSERT(beresta::xpath::selectElements(document, "//book[@lang='ru']").size() == 2);
        TEST_ASSERT_EQ(eval(document, "//book[2]/title"), "Dune");
        TEST_ASSERT_EQ(eval(document, "//book[last()]/@id"), "3");
        TEST_ASSERT_EQ(eval(document, "//book[price > 280][1]/title"), "Война и мир");
        TEST_ASSERT_EQ(eval(document, "//title[. = 'Dune']/../@id"), "2");
        TEST_ASSERT_EQ(eval(*document.getRoot(), "book[3]/preceding-sibling::book[1]/@id"), "2");
        TEST_ASSERT_EQ(eval(document, "//comment()"), " конец ");
    }

    // Оси ancestor, following, descendant и объединение
    void testAxes()
    {
        beresta::Document document = beresta::XmlParser().parse(CATALOG);
        const beresta::Element* dune = document.getElementsByTagName("title")[1];

        TEST_ASSERT_EQ(eval(*dune, "count(ancestor::*)"), "2");
        TEST_ASSERT_EQ(eval(*dune, "name(following::*[1])"), "price");
        TEST_ASSERT_EQ(eval(*dune, "count(preceding::title)"), "1");
        TEST_ASSERT_EQ(eval(document, "count(//title | //price)"), "6");
        TEST_ASSERT_EQ(eval(document, "count(/descendant::*)"), "10");
        TEST_ASSERT_EQ(eval(document, "count(//@*)"), "6");
    }

    // Пространства имён: префиксы задаются в XPathContext
    void testNamespaces()
    {
        beresta::Document document = beresta::XmlParser().parse(
            "<r xmlns=\"urn:d\" xmlns:x=\"urn:x\"><item x:k=\"1\"/><x:item/></r>");
        beresta::xpath::XPathContext context;
        context.namespaces.emplace("d", "urn:d");
        context.namespaces.emplace("x", "urn:x");

        beresta::xpath::XPathExpression items("//d:item | //x:item");
        TEST_ASSERT(items.selectElements(document, context).size() == 2);
        TEST_ASSERT(beresta::xpath::XPathExpression("//item").selectElements(document).empty());
        TEST_ASSERT_EQ(beresta::xpath::XPathExpression("string(//d:item/@x:k)").evaluate(document, context).toString(), "1");
        TEST_ASSERT_THROWS(beresta::xpath::XPathExpression("//q:item").evaluate(document), beresta::DomException);
    }
}

namespace suites
{
    // Регистрация тестов путей XPath
    void registerXPathPathTests(test::TestRunner& runner)
    {
        runner.add("XPath: пути и предикаты", testLocationPaths);
        runner.add("XPath: оси", testAxes);
        runner.add("XPath: пространства имён", testNamespaces);
    }
}
