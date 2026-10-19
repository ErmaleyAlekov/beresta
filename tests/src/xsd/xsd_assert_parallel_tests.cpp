#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace
{
    // Число потоков и проверок в каждом
    // The number of threads and of validations in each
    const int WORKERS = 8;
    const int ROUNDS = 20;

    /*
    ** Счёт с утверждением xs:assert (сумма строк) и фасетом xs:assertion
    ** (количество меньше 1000): обе ветки утверждений — составная и простая —
    ** работают на одной общей схеме из многих потоков.
    **
    ** An invoice with an xs:assert assertion (the sum of the lines) and an xs:assertion facet
    ** (the quantity is less than 1000): both kinds of assertions — complex and simple —
    ** work on one shared schema from many threads.
    */
    const char* const INVOICE_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:simpleType name=\"Quantity\"><xs:restriction base=\"xs:integer\">"
        "<xs:assertion test=\"$value lt 1000\"/></xs:restriction></xs:simpleType>"
        "<xs:element name=\"invoice\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"qty\" type=\"Quantity\" maxOccurs=\"unbounded\"/>"
        "<xs:element name=\"amount\" type=\"xs:decimal\" maxOccurs=\"unbounded\"/>"
        "<xs:element name=\"total\" type=\"xs:decimal\"/></xs:sequence>"
        "<xs:assert test=\"total = sum(amount)\"/></xs:complexType></xs:element></xs:schema>";

    // Хороший документ — без ошибок; плохой — фасет и утверждение (две ошибки)
    // A good document — no errors; a bad one — the facet and the assertion (two errors)
    const char* const GOOD_INVOICE =
        "<invoice><qty>1</qty><amount>0.10</amount><amount>0.20</amount><total>0.30</total></invoice>";
    const char* const BAD_INVOICE =
        "<invoice><qty>5000</qty><amount>0.10</amount><amount>0.20</amount><total>0.31</total></invoice>";

    /*
    ** Один поток: проверка готового дерева и потоковая проверка обоих
    ** документов ROUNDS раз; каждый неверный результат увеличивает failures
    ** (свой счётчик у каждого потока — гонки за общий счётчик нет).
    **
    ** One thread: validation of a ready tree and streaming validation of both
    ** documents ROUNDS times; each wrong result increments failures
    ** (each thread has its own counter — there is no race for a shared counter).
    */
    void validateInvoices(std::shared_ptr<const beresta::xsd::Schema> schema, int* failures)
    {
        beresta::xsd::SchemaValidator validator(schema);
        beresta::ParserOptions options;
        options.schema = schema;

        for (int round = 0; round < ROUNDS; ++round)
        {
            beresta::CollectingErrorHandler errors;
            options.errorHandler = &errors;
            if (!validator.validate(beresta::XmlParser().parse(GOOD_INVOICE)).empty())
                ++*failures;
            if (validator.validate(beresta::XmlParser().parse(BAD_INVOICE)).size() != 2)
                ++*failures;
            beresta::XmlParser(options).parse(BAD_INVOICE);
            if (errors.getDiagnostics().size() != 2)
                ++*failures;
        }
    }

    /*
    ** WORKERS потоков на одной схеме: скомпилированные утверждения
    ** неизменяемы, а буферы проверки у каждого прохода свои. В WebAssembly
    ** потоков нет — те же проверки идут по очереди.
    **
    ** WORKERS threads on one schema: the compiled assertions are
    ** immutable, and the validation buffers belong to each pass. WebAssembly
    ** has no threads — the same checks run one by one.
    */
    void testParallelValidation()
    {
        std::shared_ptr<const beresta::xsd::Schema> schema = beresta::xsd::Schema::fromString(INVOICE_XSD);
        std::vector<int> failures(WORKERS, 0);

#if defined(__EMSCRIPTEN__)
        for (int worker = 0; worker < WORKERS; ++worker)
            validateInvoices(schema, &failures[worker]);
#else
        std::vector<std::thread> threads;
        for (int worker = 0; worker < WORKERS; ++worker)
            threads.emplace_back(validateInvoices, schema, &failures[worker]);
        for (std::thread& thread : threads)
            thread.join();
#endif
        for (int count : failures)
            TEST_ASSERT(count == 0);
    }
}

namespace suites
{
    // Регистрация теста утверждений из многих потоков
    // Registers the test of assertions from many threads
    void registerXsdAssertParallelTests(test::TestRunner& runner)
    {
        runner.add("XSD 1.1: утверждения на общей схеме из 8 потоков", testParallelValidation);
    }
}
