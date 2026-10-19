#include "suites/test_suites.hpp"
#include "support/capi_helpers.hpp"

#include <chrono>
#include <string>
#include <vector>

namespace
{
    constexpr int NODES = 50000;

    /*
    ** Документ <a> с NODES детьми <n i="k">k</n>, построенный через
    ** Си-интерфейс — так, как его строили бы обёртки.
    **
    ** A document <a> with NODES children <n i="k">k</n>, built through the
    ** C interface — the way the wrappers would build it.
    */
    beresta_document makeSource()
    {
        beresta_document document = nullptr;

        beresta_document_create("a", &document);
        beresta_node root = beresta_document_root(document);
        for (int index = 0; index < NODES; ++index)
        {
            std::string number = std::to_string(index);
            beresta_node item = beresta_node_append_element(root, "n");
            beresta_element_set_attribute(item, "i", number.c_str());
            beresta_node_append_text(item, number.c_str());
        }
        return document;
    }

    /*
    ** Перенос всех детей from в конец to, начиная с последнего: так
    ** обёртки разбирают список, и каждый перенос — O(1) (индекс ребёнка
    ** хранится в узле). Порядок в to получается обратным.
    **
    ** Moves all children of from to the end of to, starting from the last one:
    ** this is how the wrappers take a list apart, and every move is O(1) (the
    ** child index is stored in the node). The order in to becomes reversed.
    */
    int moveAll(beresta_node from, beresta_node to)
    {
        int moved = 0;

        for (beresta_node child = beresta_node_last_child(from); child != nullptr; child = beresta_node_last_child(from))
        {
            if (beresta_node_move_to(child, to) != BERESTA_OK)
                break;
            ++moved;
        }
        return moved;
    }

    /*
    ** Каждый дескриптор, выданный ДО переносов, обязан указывать на тот же
    ** живой узел: владелец — target, родитель — его корень, атрибут и текст
    ** прежние, место — зеркальное. Обращение к памяти освобождённого
    ** исходного документа поймал бы AddressSanitizer.
    **
    ** Every handle given out BEFORE the moves must point to the same live node:
    ** the owner is target, the parent is its root, the attribute and the text are
    ** the same, the place is mirrored. An access to the memory of the freed source
    ** document would be caught by AddressSanitizer.
    */
    int countBrokenHandles(const std::vector<beresta_node>& handles, beresta_document target)
    {
        beresta_node root = beresta_document_root(target);
        int broken = 0;

        for (int index = 0; index < NODES; ++index)
        {
            beresta_node node = handles[static_cast<std::size_t>(index)];
            std::string number = std::to_string(index);
            bool same = beresta_node_owner_document(node) == target && beresta_node_parent(node) == root
                && beresta_node_child(root, NODES - 1 - index) == node
                && support::takeCapiString(beresta_element_get_attribute(node, "i")) == number
                && support::takeCapiString(beresta_node_text(node)) == number;
            if (!same)
                ++broken;
        }
        return broken;
    }

    /*
    ** 50 000 узлов трижды переходят между документами (A -> B -> A -> B),
    ** затем A освобождается. Раньше поиск индекса ребёнка был перебором, и
    ** три прохода стоили ~4·10⁹ сравнений; теперь — линейное время, и
    ** предел в 5 секунд оставлен с запасом даже для санитайзеров.
    **
    ** 50,000 nodes move three times between documents (A -> B -> A -> B),
    ** then A is freed. The child index lookup used to be a scan, and three passes
    ** cost ~4·10⁹ comparisons; now it is linear time, and the 5 second limit is
    ** left with a margin even for sanitizers.
    */
    void testBulkMoveBetweenDocuments()
    {
        beresta_document source = makeSource();
        beresta_document target = nullptr;
        std::vector<beresta_node> handles;

        TEST_ASSERT(beresta_document_create("b", &target) == BERESTA_OK);
        for (beresta_node child = beresta_node_first_child(beresta_document_root(source)); child != nullptr;
            child = beresta_node_next_sibling(child))
            handles.push_back(child);
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        TEST_ASSERT(moveAll(beresta_document_root(source), beresta_document_root(target)) == NODES);
        TEST_ASSERT(moveAll(beresta_document_root(target), beresta_document_root(source)) == NODES);
        TEST_ASSERT(moveAll(beresta_document_root(source), beresta_document_root(target)) == NODES);
        std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
        TEST_ASSERT(elapsed.count() < 5.0);
        TEST_ASSERT(beresta_node_child_count(beresta_document_root(source)) == 0);
        beresta_document_free(source);
        TEST_ASSERT(handles.size() == static_cast<std::size_t>(NODES));
        TEST_ASSERT_EQ(std::to_string(countBrokenHandles(handles, target)), "0");
        beresta_document_free(target);
    }
}

namespace suites
{
    // Регистрация нагрузочного теста переноса узлов
    // Registers the load test of moving nodes
    void registerCapiBulkMoveTests(test::TestRunner& runner)
    {
        runner.add("Си-интерфейс: 50 000 переносов между документами", testBulkMoveBetweenDocuments);
    }
}
