#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Фильтр: элементы <skip> пропускаются, но их дети видны; <reject> скрыт целиком
    // The filter: <skip> elements are skipped but their children are visible; <reject> is hidden whole
    class NameFilter : public beresta::NodeFilter
    {
    public:
        beresta::FilterResult acceptNode(const beresta::Node& node) const override;
    };

    // Решение по имени элемента
    // The decision by the element name
    beresta::FilterResult NameFilter::acceptNode(const beresta::Node& node) const
    {
        if (node.getType() != beresta::NodeType::ELEMENT)
            return beresta::FilterResult::ACCEPT;
        const std::string& name = static_cast<const beresta::Element&>(node).getName();

        if (name == "skip")
            return beresta::FilterResult::SKIP;
        if (name == "reject")
            return beresta::FilterResult::REJECT;
        return beresta::FilterResult::ACCEPT;
    }

    // Имена элементов, которые выдаёт TreeWalker при обходе nextNode()
    // The names of the elements TreeWalker returns when traversing with nextNode()
    std::string walkNames(beresta::Node& root, const beresta::NodeFilter* filter)
    {
        beresta::TreeWalker walker(root, beresta::show::ELEMENTS, filter);
        std::string names;

        for (beresta::Node* node = walker.nextNode(); node != nullptr; node = walker.nextNode())
            names += static_cast<beresta::Element*>(node)->getName() + " ";
        return names;
    }

    // SKIP раскрывает детей, REJECT скрывает поддерево; parentNode пропускает SKIP
    // SKIP exposes the children, REJECT hides the subtree; parentNode skips SKIP
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
        TEST_ASSERT(walker.nextSibling() == document.getRoot()->findChildElement("d"));
        TEST_ASSERT(walker.previousNode() == b);
    }

    // NodeIterator: плоский обход текста вперёд и назад
    // NodeIterator: a flat traversal of text forward and backward
    void testNodeIterator()
    {
        beresta::Document document = beresta::XmlParser().parse("<r>1<a>2</a><b>3<c>4</c></b></r>");
        NameFilter filter;
        beresta::NodeIterator iterator(document, beresta::show::TEXTS, &filter);
        std::string forward;

        for (beresta::Node* node = iterator.nextNode(); node != nullptr; node = iterator.nextNode())
            forward += node->getTextContent();
        TEST_ASSERT_EQ(forward, "1234");
        TEST_ASSERT_EQ(iterator.previousNode()->getTextContent(), "4");
        TEST_ASSERT_EQ(iterator.previousNode()->getTextContent(), "3");
    }

    // Перенесённый узел остаётся живым и обход продолжается от него.
    // The moved node stays alive and traversal continues from it.
    void testMoveCurrentNode()
    {
        beresta::Document document = beresta::XmlParser().parse("<r><old><item/></old><new><tail/></new></r>");
        beresta::Element& root = *document.getRoot();
        beresta::Element& oldParent = *root.findChildElement("old");
        beresta::Element& newParent = *root.findChildElement("new");
        beresta::Element* item = oldParent.findChildElement("item");
        beresta::TreeWalker walker(root);
        beresta::NodeIterator iterator(root);

        walker.setCurrentNode(*item);
        TEST_ASSERT(iterator.nextNode() == &root);
        TEST_ASSERT(iterator.nextNode() == &oldParent);
        TEST_ASSERT(iterator.nextNode() == item);
        newParent.appendChild(oldParent.detachChild(*item));
        TEST_ASSERT(&walker.getCurrentNode() == item);
        TEST_ASSERT(&iterator.getReferenceNode() == item);
        TEST_ASSERT(walker.previousSibling() == newParent.findChildElement("tail"));
        TEST_ASSERT(iterator.nextNode() == nullptr);
        TEST_ASSERT(iterator.previousNode() == item);
        TEST_ASSERT(iterator.previousNode() == newParent.findChildElement("tail"));
    }
}

namespace suites
{
    // Регистрация тестов обхода дерева
    // Registers the tree traversal tests
    void registerTraversalTests(test::TestRunner& runner)
    {
        runner.add("обход: TreeWalker с фильтром", testTreeWalker);
        runner.add("обход: NodeIterator", testNodeIterator);
        runner.add("обход: перенос текущего узла", testMoveCurrentNode);
    }
}
