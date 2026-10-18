#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

#include <memory>

namespace
{
    // Соседи, первый/последний ребёнок и документ-владелец
    // Siblings, the first/last child and the owner document
    void testSiblingNavigation()
    {
        beresta::Document document = beresta::XmlParser().parse("<r><a/><b/><c/></r>");
        beresta::Element* root = document.getRoot();
        beresta::Node* middle = root->getChild(1);

        TEST_ASSERT(root->getFirstChild()->getNextSibling() == middle);
        TEST_ASSERT(root->getLastChild()->getPreviousSibling() == middle);
        TEST_ASSERT(root->getFirstChild()->getPreviousSibling() == nullptr);
        TEST_ASSERT(root->getLastChild()->getNextSibling() == nullptr);
        TEST_ASSERT(middle->getOwnerDocument() == &document);
        root->removeChild(0);
        TEST_ASSERT(root->getFirstChild() == middle);
        TEST_ASSERT(middle->getPreviousSibling() == nullptr);
    }

    // Замена, отсоединение по ссылке и глубокий поиск
    // Replacement, detaching by reference and a deep search
    void testEditingAndDeepSearch()
    {
        beresta::Document document = beresta::XmlParser().parse(
            "<r><x>1</x><g><x>2</x></g></r>");
        beresta::Element* root = document.getRoot();

        TEST_ASSERT(root->getElementsByTagName("x").size() == 2);
        TEST_ASSERT(root->getElementsByTagName("*").size() == 3);
        std::unique_ptr<beresta::Node> old = root->replaceChild(0, std::make_unique<beresta::Element>("y"));
        TEST_ASSERT_EQ(old->toString(), "<x>1</x>");
        root->detachChild(*root->findChildElement("g"));
        TEST_ASSERT_EQ(document.toString(), "<r><y/></r>");
        TEST_ASSERT_THROWS(root->detachChild(*old), beresta::DomException);
    }

    // normalize: соседние тексты сливаются, пустые удаляются; setTextContent заменяет детей
    // normalize: adjacent texts are merged, empty ones removed; setTextContent replaces the children
    void testNormalizeAndTextContent()
    {
        beresta::Element element("p");
        element.appendChild(std::make_unique<beresta::Text>("a"));
        element.appendChild(std::make_unique<beresta::Text>(""));
        element.appendChild(std::make_unique<beresta::Text>("b"));

        element.normalize();
        TEST_ASSERT(element.getChildCount() == 1);
        TEST_ASSERT_EQ(element.getTextContent(), "ab");
        element.setTextContent("новый");
        TEST_ASSERT_EQ(element.toString(), "<p>новый</p>");
        element.setTextContent("");
        TEST_ASSERT_EQ(element.toString(), "<p/>");
    }

    // isEqualNode игнорирует порядок атрибутов; cloneShallow не копирует детей
    // isEqualNode ignores the attribute order; cloneShallow does not copy the children
    void testEqualityAndShallowClone()
    {
        beresta::XmlParser parser;
        beresta::Document first = parser.parse("<a x=\"1\" y=\"2\"><b>t</b></a>");
        beresta::Document second = parser.parse("<a y=\"2\" x=\"1\"><b>t</b></a>");
        beresta::Document third = parser.parse("<a y=\"2\" x=\"1\"><b>T</b></a>");

        TEST_ASSERT(first.isEqualNode(second));
        TEST_ASSERT(!first.isEqualNode(third));
        std::unique_ptr<beresta::Element> shallow = first.getRoot()->cloneShallow();
        TEST_ASSERT_EQ(shallow->toString(), "<a x=\"1\" y=\"2\"/>");
    }
}

namespace suites
{
    // Регистрация тестов навигации и редактирования
    // Registers the navigation and editing tests
    void registerNavigationTests(test::TestRunner& runner)
    {
        runner.add("навигация: соседи и владелец", testSiblingNavigation);
        runner.add("навигация: замена и глубокий поиск", testEditingAndDeepSearch);
        runner.add("навигация: normalize и setTextContent", testNormalizeAndTextContent);
        runner.add("навигация: isEqualNode и cloneShallow", testEqualityAndShallowClone);
    }
}
