#include "suites/test_suites.hpp"
#include "support/capi_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Главный сценарий: узел с потомками переносится из A в B, затем A
    ** освобождается. Дескрипторы узла и его потомка, полученные ДО переноса,
    ** обязаны остаться действительными: объект не копировался, им просто
    ** теперь владеет B (санитайзер адресов поймал бы обращение к памяти A).
    **
    ** The main scenario: a node with descendants is moved from A to B, then A
    ** is freed. The handles of the node and its descendant obtained BEFORE the
    ** move must stay valid: the object was not copied, it is simply owned by B
    ** now (an address sanitizer would catch an access to the memory of A).
    */
    void testMoveBetweenDocuments()
    {
        beresta_document source = support::parseCapi("<a><item n=\"1\"><name>Мёд</name></item><rest/></a>");
        beresta_document target = support::parseCapi("<b/>");
        beresta_node item = beresta_node_first_child(beresta_document_root(source));
        beresta_node name = beresta_node_first_child(item);

        TEST_ASSERT(beresta_node_move_to(item, beresta_document_root(target)) == BERESTA_OK);
        TEST_ASSERT(beresta_last_error_code() == BERESTA_OK);
        TEST_ASSERT_EQ(support::takeCapiString(beresta_node_to_string(source, 0)), "<a><rest/></a>");
        beresta_document_free(source);
        TEST_ASSERT(beresta_node_owner_document(item) == target);
        TEST_ASSERT(beresta_node_parent(item) == beresta_document_root(target));
        TEST_ASSERT_EQ(support::takeCapiString(beresta_node_text(name)), "Мёд");
        TEST_ASSERT(beresta_element_set_attribute(item, "n", "2") == BERESTA_OK);
        TEST_ASSERT_EQ(support::takeCapiString(beresta_node_to_string(target, 0)),
            "<b><item n=\"2\"><name>Мёд</name></item></b>");
        beresta_document_free(target);
    }

    /*
    ** Перенос внутри одного документа ставит узел в конец детей нового
    ** родителя; элемент, перенесённый в документ без корня, становится корнем.
    **
    ** A move inside one document puts the node at the end of the children of the
    ** new parent; an element moved into a document without a root becomes the root.
    */
    void testMoveInsideAndToEmptyDocument()
    {
        beresta_document document = support::parseCapi("<r><x/><y><z/></y></r>");
        beresta_document empty = nullptr;
        beresta_node root = beresta_document_root(document);
        beresta_node x = beresta_node_first_child(root);

        TEST_ASSERT(beresta_node_move_to(x, beresta_node_last_child(root)) == BERESTA_OK);
        TEST_ASSERT_EQ(support::takeCapiString(beresta_node_to_string(document, 0)), "<r><y><z/><x/></y></r>");
        TEST_ASSERT(beresta_document_create(nullptr, &empty) == BERESTA_OK);
        TEST_ASSERT(beresta_node_move_to(x, empty) == BERESTA_OK);
        TEST_ASSERT(beresta_document_root(empty) == x);
        TEST_ASSERT(beresta_node_owner_document(x) == empty);
        beresta_document_free(document);
        TEST_ASSERT_EQ(support::takeCapiString(beresta_node_to_string(empty, 0)), "<x/>");
        beresta_document_free(empty);
    }

    /*
    ** Запрещённые переносы: корень, документ, цикл, NULL, родитель-текст.
    ** Каждый даёт свой код, а дерево остаётся прежним.
    **
    ** Forbidden moves: the root, a document, a cycle, NULL, a text parent.
    ** Each gives its own code, and the tree stays the same.
    */
    void testMoveErrors()
    {
        beresta_document document = support::parseCapi("<r><p><q>t</q></p></r>");
        beresta_node root = beresta_document_root(document);
        beresta_node p = beresta_node_first_child(root);
        beresta_node q = beresta_node_first_child(p);

        TEST_ASSERT(beresta_node_move_to(root, q) == BERESTA_HIERARCHY_VIOLATION);
        TEST_ASSERT(beresta_node_move_to(p, q) == BERESTA_HIERARCHY_VIOLATION);
        TEST_ASSERT(!std::string(beresta_last_error_message()).empty());
        TEST_ASSERT(beresta_node_move_to(p, p) == BERESTA_HIERARCHY_VIOLATION);
        TEST_ASSERT(beresta_node_move_to(document, p) == BERESTA_INVALID_ARGUMENT);
        TEST_ASSERT(beresta_node_move_to(nullptr, p) == BERESTA_INVALID_ARGUMENT);
        TEST_ASSERT(beresta_node_move_to(q, nullptr) == BERESTA_INVALID_ARGUMENT);
        TEST_ASSERT(beresta_node_move_to(q, beresta_node_first_child(q)) == BERESTA_INVALID_ARGUMENT);
        TEST_ASSERT_EQ(support::takeCapiString(beresta_node_to_string(document, 0)), "<r><p><q>t</q></p></r>");
        beresta_document_free(document);
    }

    /*
    ** Новый родитель отверг узел (текст на верхнем уровне документа):
    ** узел возвращается на своё прежнее место среди соседей.
    **
    ** The new parent rejected the node (text at the top level of a document):
    ** the node goes back to its former place among its siblings.
    */
    void testMoveRejectedRestoresNode()
    {
        beresta_document source = support::parseCapi("<a><b/>текст<c/></a>");
        beresta_document target = support::parseCapi("<t/>");
        beresta_node text = beresta_node_child(beresta_document_root(source), 1);

        TEST_ASSERT(beresta_node_move_to(text, target) == BERESTA_HIERARCHY_VIOLATION);
        TEST_ASSERT_EQ(support::takeCapiString(beresta_node_to_string(source, 0)), "<a><b/>текст<c/></a>");
        TEST_ASSERT(beresta_node_child(beresta_document_root(source), 1) == text);
        TEST_ASSERT(beresta_node_owner_document(text) == source);
        TEST_ASSERT_EQ(support::takeCapiString(beresta_node_to_string(target, 0)), "<t/>");
        beresta_document_free(source);
        beresta_document_free(target);
    }
}

namespace suites
{
    // Регистрация тестов переноса узлов через Си-интерфейс
    // Registers the tests of moving nodes through the C interface
    void registerCapiMoveTests(test::TestRunner& runner)
    {
        runner.add("Си-интерфейс: перенос узла между документами", testMoveBetweenDocuments);
        runner.add("Си-интерфейс: перенос внутри документа и в пустой документ", testMoveInsideAndToEmptyDocument);
        runner.add("Си-интерфейс: запрещённые переносы", testMoveErrors);
        runner.add("Си-интерфейс: отвергнутый перенос возвращает узел", testMoveRejectedRestoresNode);
    }
}
