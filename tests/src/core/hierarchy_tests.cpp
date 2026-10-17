#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

#include <memory>
#include <utility>

namespace
{
    /*
    ** Нельзя вставить пустой указатель и узел внутрь самого себя.
    ** После неудачной вставки владение узлом остаётся у теста.
    */
    void testRejectsNullAndSelf()
    {
        std::unique_ptr<beresta::Node> empty;
        std::unique_ptr<beresta::Node> root = std::make_unique<beresta::Element>("root");
        beresta::Element& self = static_cast<beresta::Element&>(*root);

        support::AppendResult nullResult = support::tryAppend(self, empty);
        support::AppendResult selfResult = support::tryAppend(self, root);
        TEST_ASSERT(!nullResult.succeeded && nullResult.code == beresta::DomErrorCode::NULL_NODE);
        TEST_ASSERT(!selfResult.succeeded);
        TEST_ASSERT(selfResult.code == beresta::DomErrorCode::HIERARCHY_VIOLATION);
        TEST_ASSERT(root != nullptr);
    }

    // Нельзя вставить предка внутрь потомка — получился бы цикл
    void testRejectsAncestorInsertion()
    {
        std::unique_ptr<beresta::Node> root = std::make_unique<beresta::Element>("root");
        beresta::Element& rootElement = static_cast<beresta::Element&>(*root);
        beresta::Node* child = rootElement.appendChild(std::make_unique<beresta::Element>("child"));
        beresta::ContainerNode& childContainer = static_cast<beresta::ContainerNode&>(*child);

        TEST_ASSERT(child->isDescendantOf(rootElement));
        support::AppendResult result = support::tryAppend(childContainer, root);
        TEST_ASSERT(!result.succeeded);
        TEST_ASSERT(result.code == beresta::DomErrorCode::HIERARCHY_VIOLATION);
        TEST_ASSERT(root != nullptr);
    }

    // Извлечённый узел автономен и может быть вставлен в другое дерево
    void testRemovedNodeIsAutonomous()
    {
        beresta::Document first = beresta::XmlParser().parse("<a><b>text</b></a>");
        beresta::Element second("other");
        std::unique_ptr<beresta::Node> moved = first.getRoot()->removeChild(0);

        TEST_ASSERT(moved->getParent() == nullptr);
        second.appendChild(std::move(moved));
        TEST_ASSERT_EQ(first.toString(), "<a/>");
        TEST_ASSERT_EQ(second.toString(), "<other><b>text</b></other>");
    }

    // В документе только один корень; текст на верхнем уровне запрещён
    void testDocumentRules()
    {
        beresta::Document document;
        std::unique_ptr<beresta::Node> second = std::make_unique<beresta::Element>("second");
        std::unique_ptr<beresta::Node> text = std::make_unique<beresta::Text>("t");

        document.appendChild(std::make_unique<beresta::Element>("root"));
        TEST_ASSERT(!support::tryAppend(document, second).succeeded);
        TEST_ASSERT(!support::tryAppend(document, text).succeeded);
        document.setRoot(std::make_unique<beresta::Element>("replaced"));
        TEST_ASSERT_EQ(document.toString(), "<replaced/>");
    }
}

namespace suites
{
    // Регистрация тестов структуры дерева
    void registerHierarchyTests(test::TestRunner& runner)
    {
        runner.add("иерархия: null и вставка в себя", testRejectsNullAndSelf);
        runner.add("иерархия: вставка предка в потомка", testRejectsAncestorInsertion);
        runner.add("иерархия: извлечённый узел автономен", testRemovedNodeIsAutonomous);
        runner.add("иерархия: правила документа", testDocumentRules);
    }
}
