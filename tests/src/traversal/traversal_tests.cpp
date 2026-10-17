#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Фильтр: элементы <skip> пропускаются, но их дети видны; <reject> скрыт целиком
    class NameFilter : public beresta::NodeFilter
    {
    public:
        beresta::FilterResult acceptNode(const beresta::Node& node) const override;
    };

    // Решение по имени элемента
    beresta::FilterResult NameFilter::acceptNode(const beresta::Node& node) const
    {
        const std::string& name = static_cast<const beresta::Element&>(node).getName();

        if (name == "skip")
            return beresta::FilterResult::SKIP;
        if (name == "reject")
            return beresta::FilterResult::REJECT;
        return beresta::FilterResult::ACCEPT;
    }

    // Имена элементов, которые выдаёт TreeWalker при обходе nextNode()
    std::string walkNames(beresta::Node& root, const beresta::NodeFilter* filter)
    {
        beresta::TreeWalker walker(root, beresta::show::ELEMENTS, filter);
        std::string names;

        for (beresta::Node* node = walker.nextNode(); node != nullptr; node = walker.nextNode())
            names += static_cast<beresta::Element*>(node)->getName() + " ";
        return names;
    }

    // SKIP раскрывает детей, REJECT скрывает поддерево; parentNode пропускает SKIP
    void testTreeWalker()
    {
        beresta::Document document = beresta::XmlParser().parse(
            "<r><a/><skip><b/></skip><reject><c/></reject><d/></r>");
        NameFilter filter;
        beresta::TreeWalker walker(*document.getRoot(), beresta::show::ELEMENTS, &filter);

        TEST_ASSERT_EQ(walkNames(*document.getRoot(), &filter), "a b d ");
        TEST_ASSERT_EQ(walkNames(*document.getRoot(), nullptr), "a skip b reject c d ");
        beresta::Node* b = document.getElementsByTagName("b").front();
        walker.setCurrentNode(*b);
        TEST_ASSERT(walker.parentNode() == document.getRoot());
        walker.setCurrentNode(*b);
        TEST_ASSERT_EQ(static_cast<beresta::Element*>(walker.nextSibling())->getName(), "d");
        TEST_ASSERT_EQ(static_cast<beresta::Element*>(walker.previousNode())->getName(), "b");
    }

    // NodeIterator: плоский обход текста вперёд и назад
    void testNodeIterator()
    {
        beresta::Document document = beresta::XmlParser().parse("<r>1<a>2</a><b>3<c>4</c></b></r>");
        beresta::NodeIterator iterator(document, beresta::show::TEXTS);
        std::string forward;

        for (beresta::Node* node = iterator.nextNode(); node != nullptr; node = iterator.nextNode())
            forward += node->getTextContent();
        TEST_ASSERT_EQ(forward, "1234");
        TEST_ASSERT_EQ(iterator.previousNode()->getTextContent(), "4");
        TEST_ASSERT_EQ(iterator.previousNode()->getTextContent(), "3");
    }
}

namespace suites
{
    // Регистрация тестов обхода дерева
    void registerTraversalTests(test::TestRunner& runner)
    {
        runner.add("обход: TreeWalker с фильтром", testTreeWalker);
        runner.add("обход: NodeIterator", testNodeIterator);
    }
}
