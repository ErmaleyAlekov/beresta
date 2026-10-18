#include "suites/test_suites.hpp"
#include "support/capi_helpers.hpp"

#include <cmath>
#include <cstring>
#include <string>

namespace
{
    /*
    ** Коды ошибок и сообщения: синтаксическая ошибка, NULL вместо узла,
    ** узел не того типа; успешный вызов сбрасывает последнюю ошибку.
    **
    ** Error codes and messages: a syntax error, NULL instead of a node,
    ** a node of the wrong type; a successful call resets the last error.
    */
    void testErrors()
    {
        beresta_document document = nullptr;

        TEST_ASSERT(beresta_document_parse("<a><b></a>", 10, 0, nullptr, &document) == BERESTA_PARSE_ERROR);
        TEST_ASSERT(document == nullptr);
        TEST_ASSERT(std::string(beresta_last_error_message()).find("строка 1") == 0);
        TEST_ASSERT_EQ(std::string(beresta_status_name(BERESTA_PARSE_ERROR)), "PARSE_ERROR");
        TEST_ASSERT(beresta_node_type(nullptr) == -1);
        TEST_ASSERT(beresta_last_error_code() == BERESTA_INVALID_ARGUMENT);
        document = support::parseCapi("<a>текст</a>");
        TEST_ASSERT(beresta_last_error_code() == BERESTA_OK);
        TEST_ASSERT(beresta_element_get_attribute(beresta_node_first_child(beresta_document_root(document)), "x") == nullptr);
        TEST_ASSERT(beresta_last_error_code() == BERESTA_INVALID_ARGUMENT);
        TEST_ASSERT(beresta_node_append_element(beresta_document_root(document), "1bad") == nullptr);
        TEST_ASSERT(beresta_last_error_code() == BERESTA_INVALID_NAME);
        beresta_document_free(document);
    }

    // Построение, навигация, атрибуты, копирование между документами, удаление
    // Building, navigation, attributes, copying between documents, removal
    void testTreeEditing()
    {
        beresta_document document = nullptr;
        beresta_document other = support::parseCapi("<copy/>");

        TEST_ASSERT(beresta_document_create("order", &document) == BERESTA_OK);
        beresta_node root = beresta_document_root(document);
        beresta_node item = beresta_node_append_element(root, "item");
        TEST_ASSERT(beresta_element_set_attribute(item, "qty", "2") == BERESTA_OK);
        beresta_node_append_text(item, "Мёд & орехи");
        beresta_node_append_comment(root, " итог ");
        TEST_ASSERT_EQ(support::takeCapiString(beresta_node_to_string(document, 0)),
            "<order><item qty=\"2\">Мёд &amp; орехи</item><!-- итог --></order>");
        TEST_ASSERT(beresta_node_child_count(root) == 2);
        TEST_ASSERT(beresta_node_next_sibling(item) == beresta_node_last_child(root));
        TEST_ASSERT(beresta_node_owner_document(item) == document);
        TEST_ASSERT_EQ(support::takeCapiString(beresta_element_attribute_name(item, 0)), "qty");
        TEST_ASSERT(beresta_node_copy_to(item, beresta_document_root(other), nullptr) == BERESTA_OK);
        TEST_ASSERT(beresta_node_remove(item) == BERESTA_OK);
        TEST_ASSERT_EQ(support::takeCapiString(beresta_node_to_string(other, 0)), "<copy><item qty=\"2\">Мёд &amp; орехи</item></copy>");
        TEST_ASSERT_EQ(support::takeCapiString(beresta_node_name(beresta_node_first_child(root))), "#comment");
        beresta_document_free(other);
        beresta_document_free(document);
    }

    // XPath: префиксы, переменные, строка/число/логическое, набор с атрибутами
    // XPath: prefixes, variables, string/number/boolean, a set with attributes
    void testXPath()
    {
        beresta_document document = support::parseCapi("<s:shop xmlns:s=\"urn:s\"><s:item code=\"A\">5</s:item>"
            "<s:item code=\"B\">7</s:item></s:shop>");
        beresta_xpath xpath = nullptr;
        beresta_nodeset nodeset = nullptr;
        double total = 0;

        TEST_ASSERT(beresta_xpath_compile("sum(//p:item[@code != $skip])", &xpath) == BERESTA_OK);
        beresta_xpath_bind_namespace(xpath, "p", "urn:s");
        beresta_xpath_set_string(xpath, "skip", "A");
        TEST_ASSERT(beresta_xpath_number(xpath, document, &total) == BERESTA_OK);
        TEST_ASSERT(std::fabs(total - 7) < 1e-9);
        beresta_xpath_free(xpath);
        beresta_xpath_compile("//@code", &xpath);
        TEST_ASSERT(beresta_xpath_select(xpath, document, &nodeset) == BERESTA_OK);
        TEST_ASSERT(beresta_nodeset_size(nodeset) == 2);
        TEST_ASSERT(beresta_nodeset_node(nodeset, 1) == nullptr);
        TEST_ASSERT_EQ(support::takeCapiString(beresta_nodeset_value(nodeset, 1)), "B");
        TEST_ASSERT(beresta_nodeset_value(nodeset, 2) == nullptr);
        beresta_nodeset_free(nodeset);
        beresta_xpath_free(xpath);
        TEST_ASSERT(beresta_xpath_compile("//[", &xpath) == BERESTA_XPATH_ERROR);
        beresta_document_free(document);
    }

    // Схема: проверка с перечнем нарушений и проверка прямо при разборе
    // A schema: validation with a list of violations and validation right during parsing
    void testSchema()
    {
        const char* xsd = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
            "<xs:element name=\"n\" type=\"xs:int\"/></xs:schema>";
        beresta_schema schema = nullptr;
        beresta_diagnostics diagnostics = nullptr;
        beresta_document document = nullptr;

        TEST_ASSERT(beresta_schema_load(xsd, static_cast<int64_t>(std::strlen(xsd)), &schema) == BERESTA_OK);
        beresta_document bad = support::parseCapi("<n>abc</n>");
        TEST_ASSERT(beresta_schema_validate(schema, bad, &diagnostics) == BERESTA_OK);
        TEST_ASSERT(beresta_diagnostics_count(diagnostics) == 1);
        TEST_ASSERT(!support::takeCapiString(beresta_diagnostics_message(diagnostics, 0)).empty());
        TEST_ASSERT(beresta_document_parse("<n>12</n>", 9, 0, schema, &document) == BERESTA_OK);
        TEST_ASSERT(beresta_document_parse("<n>x</n>", 8, 0, schema, &bad) == BERESTA_VALIDATION_ERROR);
        beresta_diagnostics_free(diagnostics);
        beresta_document_free(document);
        beresta_document_free(bad);
        beresta_schema_free(schema);
    }
}

namespace suites
{
    // Регистрация тестов плоского Си-интерфейса
    // Registers the tests of the flat C interface
    void registerCapiTests(test::TestRunner& runner)
    {
        runner.add("Си-интерфейс: коды ошибок", testErrors);
        runner.add("Си-интерфейс: построение и изменение дерева", testTreeEditing);
        runner.add("Си-интерфейс: XPath", testXPath);
        runner.add("Си-интерфейс: XML Schema", testSchema);
    }
}
