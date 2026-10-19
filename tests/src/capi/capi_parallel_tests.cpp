#include "suites/test_suites.hpp"
#include "support/capi_helpers.hpp"

#include <atomic>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

namespace
{
    constexpr int WORKERS = 20;  // потоков, как у пула серверных обработчиков / threads, like a pool of server handlers
    constexpr int ROUNDS = 30;   // документов на поток / documents per thread

    /*
    ** Общее для всех потоков и только для чтения: одна схема, одно
    ** скомпилированное выражение XPath и один документ. Так обёртки
    ** работают в сервере: схема и выражения загружаются при старте и
    ** делятся между запросами. Потоки только считают свои расхождения.
    **
    ** Shared by all threads and read-only: one schema, one compiled XPath
    ** expression and one document. This is how the wrappers work in a server:
    ** the schema and the expressions are loaded at startup and shared between
    ** requests. The threads only count their mismatches.
    */
    struct SharedState
    {
        beresta_schema schema;
        beresta_xpath xpath;
        beresta_document shared;
        std::atomic<int> failures;
    };

    /*
    ** Один запрос: свой документ из тех же байтов, XPath по нему и по
    ** общему документу, проверка дерева и потоковая проверка общей схемой.
    ** Затем — заведомо неверный вызов: код ошибки хранится у каждого потока
    ** свой (thread_local), и чётные потоки обязаны увидеть XPATH_ERROR,
    ** а нечётные — PARSE_ERROR, сколько бы ошибок ни делали соседи.
    **
    ** One request: an own document from the same bytes, XPath over it and over
    ** the shared document, tree validation and streaming validation with the shared
    ** schema. Then a deliberately wrong call: every thread keeps its own error code
    ** (thread_local), and even threads must see XPATH_ERROR, while odd ones see
    ** PARSE_ERROR, no matter how many errors the neighbors make.
    */
    bool serveRequest(SharedState& state, int worker)
    {
        int64_t size = static_cast<int64_t>(std::strlen(support::ORDER_XML));
        beresta_document own = support::parseCapi(support::ORDER_XML);
        beresta_diagnostics tree = nullptr;
        beresta_diagnostics stream = nullptr;
        bool good = support::takeCapiString(beresta_xpath_string(state.xpath, own)) == "3:кофе"
            && support::takeCapiString(beresta_xpath_string(state.xpath, state.shared)) == "3:кофе"
            && beresta_schema_validate(state.schema, own, &tree) == BERESTA_OK
            && beresta_schema_validate_bytes(support::ORDER_XML, size, 0, state.schema, &stream) == BERESTA_OK;

        good = good && support::takeCapiDiagnostics(tree).size() == 2 && support::takeCapiDiagnostics(stream).size() == 2;
        beresta_document_free(own);
        beresta_xpath broken = nullptr;
        beresta_document wrong = nullptr;
        int expected = BERESTA_OK;
        if (worker % 2 == 0)
            expected = beresta_xpath_compile("((", &broken);
        else
            expected = beresta_document_parse("<a>", 3, BERESTA_PARSE_DEFAULT, nullptr, &wrong);
        std::this_thread::yield();
        bool isolated = expected != BERESTA_OK && beresta_last_error_code() == expected;
        beresta_document_free(wrong);
        beresta_xpath_free(broken);
        return good && isolated;
    }

    // Тело потока: ROUNDS запросов подряд / The thread body: ROUNDS requests in a row
    void runWorker(SharedState* state, int worker)
    {
        for (int round = 0; round < ROUNDS; ++round)
        {
            if (!serveRequest(*state, worker))
                ++state->failures;
        }
    }

    /*
    ** Запуск WORKERS потоков. WebAssembly собирается без потоков
    ** (std::thread там бросает исключение), поэтому в нём те же запросы
    ** выполняются по очереди: проверяется хотя бы логика.
    **
    ** Starts WORKERS threads. WebAssembly is built without threads
    ** (std::thread throws an exception there), so the same requests run one
    ** by one in it: at least the logic is tested.
    */
    void runWorkers(SharedState& state)
    {
#if defined(__EMSCRIPTEN__)
        for (int worker = 0; worker < WORKERS; ++worker)
            runWorker(&state, worker);
#else
        std::vector<std::thread> threads;

        for (int worker = 0; worker < WORKERS; ++worker)
            threads.emplace_back(runWorker, &state, worker);
        for (std::thread& thread : threads)
            thread.join();
#endif
    }

    /*
    ** 20 потоков x 30 запросов на одних и тех же дескрипторах схемы, XPath
    ** и документа. Неизменяемость проверяется результатом: каждый ответ
    ** совпадает с однопоточным, а под ThreadSanitizer (make ... -fsanitize=thread)
    ** тест не даёт ни одного отчёта о гонке.
    **
    ** 20 threads x 30 requests on the same schema, XPath and document handles.
    ** Immutability is checked by the result: every answer matches the
    ** single-threaded one, and under ThreadSanitizer (make ... -fsanitize=thread)
    ** the test produces no race report.
    */
    void testParallelSharedHandles()
    {
        SharedState state;

        state.schema = support::loadCapiSchema(support::ORDER_XSD);
        state.shared = support::parseCapi(support::ORDER_XML);
        state.xpath = nullptr;
        state.failures = 0;
        TEST_ASSERT(beresta_xpath_compile("concat(count(//item), ':', //item[2])", &state.xpath) == BERESTA_OK);
        runWorkers(state);
        TEST_ASSERT_EQ(std::to_string(state.failures.load()), "0");
        TEST_ASSERT_EQ(support::takeCapiString(beresta_xpath_string(state.xpath, state.shared)), "3:кофе");
        beresta_xpath_free(state.xpath);
        beresta_document_free(state.shared);
        beresta_schema_free(state.schema);
    }
}

namespace suites
{
    // Регистрация многопоточных тестов Си-интерфейса
    // Registers the multithreaded tests of the C interface
    void registerCapiParallelTests(test::TestRunner& runner)
    {
        runner.add("Си-интерфейс: 20 потоков на общих схеме, XPath и документе", testParallelSharedHandles);
    }
}
