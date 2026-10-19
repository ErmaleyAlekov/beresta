#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include "capi/beresta_c.h"

#include <cstdint>
#include <string>

namespace
{
    constexpr std::size_t DEPTH = 10000;

    /*
    ** Рекурсивная цепочка: элемент n имеет именованный тип Node, а тот
    ** снова ссылается на n. У каждого уровня обязательный атрибут level, а
    ** xs:unique на корне требует, чтобы все 10 000 значений были разными.
    ** Так одновременно нагружаются стек моделей содержимого, стек типов и
    ** таблица ключей потокового валидатора.
    **
    ** A recursive chain: the element n has the named type Node, which refers
    ** to n again. Every level has a required level attribute, and xs:unique on
    ** the root requires all 10,000 values to be distinct. This loads the content
    ** model stack, the type stack and the key table of the streaming validator at once.
    */
    const char* const DEEP_XSD =
        "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
        "<xs:complexType name='Node'><xs:sequence><xs:element ref='n' minOccurs='0'/></xs:sequence>"
        "<xs:attribute name='level' type='xs:nonNegativeInteger' use='required'/></xs:complexType>"
        "<xs:element name='n' type='Node'/>"
        "<xs:element name='root'><xs:complexType><xs:sequence><xs:element ref='n'/></xs:sequence></xs:complexType>"
        "<xs:unique name='levels'><xs:selector xpath='.//n'/><xs:field xpath='@level'/></xs:unique>"
        "</xs:element></xs:schema>";

    /*
    ** <root><n level='0'><n level='1'>…</n></n></root> глубиной DEPTH.
    ** broken — две ошибки: на середине цепочки level='x' (не число), а на
    ** самом дне — повтор level='0' (нарушение xs:unique).
    **
    ** <root><n level='0'><n level='1'>…</n></n></root> of depth DEPTH.
    ** broken — two errors: in the middle of the chain level='x' (not a number),
    ** and at the very bottom a repeated level='0' (an xs:unique violation).
    */
    std::string deepChain(bool broken)
    {
        std::string xml = "<root>";

        xml.reserve(DEPTH * 24);
        for (std::size_t level = 0; level < DEPTH; ++level)
        {
            std::string value = std::to_string(level);
            if (broken && level == DEPTH / 2)
                value = "x";
            if (broken && level == DEPTH - 1)
                value = "0";
            xml += "<n level='" + value + "'>";
        }
        for (std::size_t level = 0; level < DEPTH; ++level)
            xml += "</n>";
        return xml + "</root>";
    }

    /*
    ** Корректная цепочка в 10 000 уровней: SAX без дерева, разбор в DOM со
    ** схемой и проверка готового дерева дают пустой список. Обход везде по
    ** явному стеку: рекурсия глубиной 10 000 переполнила бы стек потока
    ** (1 МБ на Windows) и уронила бы процесс, а не тест.
    **
    ** A valid chain of 10,000 levels: SAX without a tree, parsing into a DOM with
    ** a schema and validation of a ready tree give an empty list. The walk is by
    ** an explicit stack everywhere: a recursion 10,000 deep would overflow the
    ** thread stack (1 MB on Windows) and crash the process, not the test.
    */
    void testDeepRecursiveSchemaValid()
    {
        beresta::ParserOptions options;

        options.maxDepth = DEPTH + 2;
        support::SchemaRuns runs = support::runSchemaModes(DEEP_XSD, deepChain(false), options);
        TEST_ASSERT(runs.sax.empty());
        TEST_ASSERT(runs.parsed.empty());
        TEST_ASSERT(runs.tree.empty());
    }

    /*
    ** Две ошибки на глубине 5 000 и 10 000: все три режима находят одни и те
    ** же, в одном порядке, с одинаковыми строками и столбцами; путь второй
    ** ошибки проходит через все 10 000 шагов.
    **
    ** Two errors at the depths of 5,000 and 10,000: all three modes find the same
    ** ones, in the same order, with the same lines and columns; the path of the
    ** second error goes through all 10,000 steps.
    */
    void testDeepRecursiveSchemaErrors()
    {
        beresta::ParserOptions options;

        options.maxDepth = DEPTH + 2;
        support::SchemaRuns runs = support::runSchemaModes(DEEP_XSD, deepChain(true), options);
        TEST_ASSERT_EQ(std::to_string(runs.tree.size()), "2");
        TEST_ASSERT(support::sameDiagnostics(runs.sax, runs.parsed));
        TEST_ASSERT(support::sameDiagnostics(runs.sax, runs.tree));
        TEST_ASSERT(runs.tree.front().message.find("/n/n/n") != std::string::npos);
    }

    /*
    ** Из обёрток лимит глубины по умолчанию (1024) не снимается: тот же
    ** документ через Си-интерфейс — аккуратный отказ LIMIT_EXCEEDED, а не
    ** переполнение стека. Защита от «бомб глубины» остаётся включённой.
    **
    ** The default depth limit (1024) is not lifted from the wrappers: the same
    ** document through the C interface is a clean LIMIT_EXCEEDED refusal, not a
    ** stack overflow. The protection against "depth bombs" stays on.
    */
    void testDeepDocumentLimitInCapi()
    {
        std::string xml = deepChain(false);
        beresta_schema schema = nullptr;
        beresta_diagnostics out = nullptr;

        TEST_ASSERT(beresta_schema_load(DEEP_XSD, static_cast<int64_t>(std::string(DEEP_XSD).size()), &schema) == BERESTA_OK);
        TEST_ASSERT(beresta_schema_validate_bytes(xml.data(), static_cast<int64_t>(xml.size()),
            BERESTA_PARSE_DEFAULT, schema, &out) == BERESTA_LIMIT_EXCEEDED);
        TEST_ASSERT(out == nullptr);
        beresta_schema_free(schema);
    }
}

namespace suites
{
    // Регистрация тестов глубокой рекурсивной схемы
    // Registers the deep recursive schema tests
    void registerXsdDeepTests(test::TestRunner& runner)
    {
        runner.add("XSD: рекурсивная схема, 10 000 уровней без ошибок", testDeepRecursiveSchemaValid);
        runner.add("XSD: рекурсивная схема, ошибки на глубине 10 000", testDeepRecursiveSchemaErrors);
        runner.add("XSD: глубина 10 000 из Си-интерфейса упирается в лимит", testDeepDocumentLimitInCapi);
    }
}
