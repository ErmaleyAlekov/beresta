#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

#include <memory>

namespace
{
    // Атрибуты: добавление, замена с сохранением порядка, удаление
    // Attributes: adding, replacing keeping the order, removing
    void testAttributes()
    {
        beresta::Element element("item");

        element.setAttribute("a", "1");
        element.setAttribute("b", "2");
        element.setAttribute("a", "3");
        TEST_ASSERT_EQ(element.toString(), "<item a=\"3\" b=\"2\"/>");
        TEST_ASSERT(element.getAttribute("b").value() == "2");
        TEST_ASSERT(!element.getAttribute("missing").has_value());
        TEST_ASSERT(element.removeAttribute("a"));
        TEST_ASSERT(!element.removeAttribute("a"));
        TEST_ASSERT_EQ(element.toString(), "<item b=\"2\"/>");
    }

    // Поиск дочерних элементов по имени (только непосредственные дети)
    // Looking up child elements by name (only direct children)
    void testFindChildren()
    {
        beresta::Document document = beresta::XmlParser().parse(
            "<r><x>1</x><y/><x>2</x><z><x>deep</x></z></r>");
        const beresta::Element& root = *document.getRoot();

        TEST_ASSERT(root.findChildElements("x").size() == 2);
        TEST_ASSERT_EQ(root.findChildElement("x")->getTextContent(), "1");
        TEST_ASSERT(root.findChildElement("missing") == nullptr);
    }

    // Текст элемента собирается из всех потомков, без комментариев
    // The text of an element is collected from all descendants, without comments
    void testTextContent()
    {
        beresta::Document document = beresta::XmlParser().parse(
            "<p>Hello, <b>world</b><!-- skip --><![CDATA[!]]></p>");

        TEST_ASSERT_EQ(document.getRoot()->getTextContent(), "Hello, world!");
    }

    // Недопустимые имена отвергаются с кодом INVALID_NAME
    // Invalid names are rejected with the INVALID_NAME code
    void testInvalidNames()
    {
        beresta::Element element("ok");

        TEST_ASSERT_THROWS(beresta::Element("1bad"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::Element(""), beresta::DomException);
        TEST_ASSERT_THROWS(element.setAttribute("a b", "v"), beresta::DomException);
        TEST_ASSERT_THROWS(element.setName("x<y"), beresta::DomException);
        beresta::Element cyrillic("элемент");
        TEST_ASSERT_EQ(cyrillic.toString(), "<элемент/>");
    }
}

namespace suites
{
    // Регистрация тестов элемента
    // Registers the element tests
    void registerElementTests(test::TestRunner& runner)
    {
        runner.add("элемент: атрибуты", testAttributes);
        runner.add("элемент: поиск детей", testFindChildren);
        runner.add("элемент: текстовое содержимое", testTextContent);
        runner.add("элемент: проверка имён", testInvalidNames);
    }
}
