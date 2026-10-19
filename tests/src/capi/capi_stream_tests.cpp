#include "suites/test_suites.hpp"
#include "support/capi_helpers.hpp"
#include "support/dom_helpers.hpp"

#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
    /*
    ** Потоковая проверка обязана давать ровно те же нарушения, что и
    ** проверка готового дерева: те же тексты с путями, те же строки и
    ** столбцы (дерево разобрано с BERESTA_PARSE_TRACK_LOCATIONS; без флага
    ** у узлов нет позиций, и места нарушений — 0). Корректный документ —
    ** пустой список, кодировка windows-1251 определяется по декларации.
    **
    ** Streaming validation must give exactly the same violations as the
    ** validation of a ready tree: the same texts with paths, the same lines and
    ** columns (the tree is parsed with BERESTA_PARSE_TRACK_LOCATIONS; without the
    ** flag nodes have no positions, and the places of violations are 0). A valid
    ** document gives an empty list, the windows-1251 encoding is detected by the declaration.
    */
    void testStreamMatchesTree()
    {
        beresta_schema schema = support::loadCapiSchema(support::ORDER_XSD);
        beresta_document document = support::parseCapi(support::ORDER_XML, BERESTA_PARSE_TRACK_LOCATIONS);
        beresta_diagnostics tree = nullptr;
        beresta_diagnostics stream = nullptr;
        const char* valid = "<?xml version='1.0' encoding='windows-1251'?><order><item qty='1'>\xD7\xE0\xE9</item></order>";

        TEST_ASSERT(beresta_schema_validate(schema, document, &tree) == BERESTA_OK);
        TEST_ASSERT(beresta_schema_validate_bytes(support::ORDER_XML, static_cast<int64_t>(std::strlen(support::ORDER_XML)),
            BERESTA_PARSE_DEFAULT, schema, &stream) == BERESTA_OK);
        std::vector<std::string> expected = support::takeCapiDiagnostics(tree);
        TEST_ASSERT(expected.size() == 2 && expected[0].compare(0, 4, "3:1:") == 0);
        TEST_ASSERT(support::takeCapiDiagnostics(stream) == expected);
        TEST_ASSERT(beresta_schema_validate_bytes(valid, static_cast<int64_t>(std::strlen(valid)),
            BERESTA_PARSE_DEFAULT, schema, &stream) == BERESTA_OK);
        TEST_ASSERT(support::takeCapiDiagnostics(stream).empty());
        beresta_document_free(document);
        beresta_schema_free(schema);
    }

    /*
    ** Файл: кириллическое имя (путь в UTF-8), те же нарушения, что у байтов.
    ** Пустой файл отобразить нельзя — срабатывает обычное чтение, и он
    ** честно не разбирается (нет корня). Отсутствующий файл — IO_ERROR.
    ** При ошибке out не трогается.
    **
    ** A file: a Cyrillic name (a UTF-8 path), the same violations as for bytes.
    ** An empty file cannot be mapped — ordinary reading takes over, and it
    ** honestly fails to parse (no root). A missing file is an IO_ERROR.
    ** On error out is not touched.
    */
    void testStreamFile()
    {
        std::filesystem::path directory = support::makeTestDirectory("capi_stream");
        std::filesystem::path file = directory / std::filesystem::u8path("выгрузка.xml");
        std::string empty = (directory / "empty.xml").u8string();
        std::string missing = (directory / "missing.xml").u8string();
        beresta_schema schema = support::loadCapiSchema(support::ORDER_XSD);
        beresta_diagnostics out = nullptr;

        support::writeTestFile(file, support::ORDER_XML);
        support::writeTestFile(directory / "empty.xml", "");
        TEST_ASSERT(beresta_schema_validate_file(file.u8string().c_str(), BERESTA_PARSE_DEFAULT, schema, &out) == BERESTA_OK);
        std::vector<std::string> problems = support::takeCapiDiagnostics(out);
        TEST_ASSERT(problems.size() == 2);
        TEST_ASSERT(problems[0].find("/order/item[2]") != std::string::npos);
        out = nullptr;
        TEST_ASSERT(beresta_schema_validate_file(empty.c_str(), BERESTA_PARSE_DEFAULT, schema, &out) == BERESTA_PARSE_ERROR);
        TEST_ASSERT(beresta_schema_validate_file(missing.c_str(), BERESTA_PARSE_DEFAULT, schema, &out) == BERESTA_IO_ERROR);
        TEST_ASSERT(out == nullptr);
        beresta_schema_free(schema);
    }

    /*
    ** Нарушение схемы — не ошибка вызова, а синтаксическая ошибка XML —
    ** ошибка: дальше проверять нечего. Без схемы, без out и с
    ** отрицательной длиной — INVALID_ARGUMENT с текстом ошибки.
    **
    ** A schema violation is not a call error, while an XML syntax error is an
    ** error: there is nothing further to validate. Without a schema, without out
    ** and with a negative length — INVALID_ARGUMENT with an error text.
    */
    void testStreamErrors()
    {
        beresta_schema schema = support::loadCapiSchema(support::ORDER_XSD);
        beresta_diagnostics out = nullptr;
        const char* broken = "<order><item qty='1'></order>";
        int64_t size = static_cast<int64_t>(std::strlen(broken));

        TEST_ASSERT(beresta_schema_validate_bytes(broken, size, BERESTA_PARSE_DEFAULT, schema, &out) == BERESTA_PARSE_ERROR);
        TEST_ASSERT(out == nullptr);
        TEST_ASSERT(!std::string(beresta_last_error_message()).empty());
        TEST_ASSERT(beresta_schema_validate_bytes(broken, size, BERESTA_PARSE_DEFAULT, nullptr, &out) == BERESTA_INVALID_ARGUMENT);
        TEST_ASSERT(beresta_schema_validate_bytes(broken, size, BERESTA_PARSE_DEFAULT, schema, nullptr) == BERESTA_INVALID_ARGUMENT);
        TEST_ASSERT(beresta_schema_validate_bytes(broken, -1, BERESTA_PARSE_DEFAULT, schema, &out) == BERESTA_INVALID_ARGUMENT);
        TEST_ASSERT(beresta_schema_validate_file(nullptr, BERESTA_PARSE_DEFAULT, schema, &out) == BERESTA_INVALID_ARGUMENT);
        TEST_ASSERT(out == nullptr);
        beresta_schema_free(schema);
    }
}

namespace suites
{
    // Регистрация тестов потоковой проверки через Си-интерфейс
    // Registers the tests of streaming validation through the C interface
    void registerCapiStreamTests(test::TestRunner& runner)
    {
        runner.add("Си-интерфейс: потоковая проверка совпадает с проверкой дерева", testStreamMatchesTree);
        runner.add("Си-интерфейс: потоковая проверка файла", testStreamFile);
        runner.add("Си-интерфейс: ошибки потоковой проверки", testStreamErrors);
    }
}
