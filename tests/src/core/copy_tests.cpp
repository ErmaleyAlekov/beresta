#include "suites/test_suites.hpp"
#include "beresta.hpp"

#include <memory>
#include <utility>

namespace
{
    // Копия глубокая: изменения копии не затрагивают оригинал, копия автономна
    // A copy is deep: changes to the copy do not affect the original, the copy is standalone
    void testCopyConstructorIsDeep()
    {
        beresta::Document document = beresta::XmlParser().parse("<a><b x=\"1\"><c/></b></a>");
        beresta::Element* original = document.getRoot()->findChildElement("b");
        beresta::Element copy = *original;

        copy.setAttribute("x", "2");
        copy.findChildElement("c")->setName("d");
        TEST_ASSERT(copy.getParent() == nullptr);
        TEST_ASSERT(copy.getChild(0)->getParent() == &copy);
        TEST_ASSERT_EQ(original->toString(), "<b x=\"1\"><c/></b>");
        TEST_ASSERT_EQ(copy.toString(), "<b x=\"2\"><d/></b>");
    }

    // clone() через базовый указатель сохраняет реальный тип узла
    // clone() through a base pointer keeps the real node type
    void testCloneKeepsDynamicType()
    {
        std::unique_ptr<beresta::Node> text = std::make_unique<beresta::Text>("t");
        std::unique_ptr<beresta::Node> comment = std::make_unique<beresta::Comment>("c");
        beresta::Document document = beresta::XmlParser().parse("<?pi data?><r>x</r>");
        std::unique_ptr<beresta::Node> documentClone = document.clone();

        TEST_ASSERT(text->clone()->getType() == beresta::NodeType::TEXT);
        TEST_ASSERT(comment->clone()->getType() == beresta::NodeType::COMMENT);
        TEST_ASSERT(documentClone->getType() == beresta::NodeType::DOCUMENT);
        TEST_ASSERT_EQ(documentClone->toString(), "<?pi data?><r>x</r>");
    }

    // При перемещении дети перепривязываются к новому владельцу
    // On moving the children are rebound to the new owner
    void testMoveReparentsChildren()
    {
        beresta::Element source("root");
        source.appendChild(std::make_unique<beresta::Element>("child"));
        beresta::Element target(std::move(source));

        TEST_ASSERT(target.getChildCount() == 1);
        TEST_ASSERT(target.getChild(0)->getParent() == &target);
    }

    // Присваивание меняет содержимое, но сохраняет место узла в дереве
    // Assignment changes the content but keeps the node's place in the tree
    void testAssignmentKeepsTreePosition()
    {
        beresta::Document document = beresta::XmlParser().parse("<a><b/></a>");
        beresta::Element* inner = document.getRoot()->findChildElement("b");
        beresta::Element replacement("z");
        replacement.appendChild(std::make_unique<beresta::Text>("new"));

        *inner = replacement;
        TEST_ASSERT(inner->getParent() == document.getRoot());
        TEST_ASSERT_EQ(document.toString(), "<a><z>new</z></a>");
        TEST_ASSERT_EQ(replacement.toString(), "<z>new</z>");
    }
}

namespace suites
{
    // Регистрация тестов копирования
    // Registers the copy tests
    void registerCopyTests(test::TestRunner& runner)
    {
        runner.add("копирование: конструктор копирования глубокий", testCopyConstructorIsDeep);
        runner.add("копирование: clone() сохраняет тип", testCloneKeepsDynamicType);
        runner.add("копирование: перемещение перепривязывает детей", testMoveReparentsChildren);
        runner.add("копирование: присваивание сохраняет позицию", testAssignmentKeepsTreePosition);
    }
}
