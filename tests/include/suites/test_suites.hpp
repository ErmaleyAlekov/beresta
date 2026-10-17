#ifndef TEST_SUITES_HPP
#define TEST_SUITES_HPP

#include "framework/test_framework.hpp"

namespace suites
{
    // Каждая функция регистрирует в раннере тесты своего набора

    // ---- Ядро ----
    void registerCopyTests(test::TestRunner& runner);
    void registerHierarchyTests(test::TestRunner& runner);
    void registerElementTests(test::TestRunner& runner);
    void registerSerializeTests(test::TestRunner& runner);
    void registerNavigationTests(test::TestRunner& runner);
    void registerNamespaceTests(test::TestRunner& runner);

    // ---- Парсер ----
    void registerFormatTests(test::TestRunner& runner);
    void registerContentTests(test::TestRunner& runner);
    void registerErrorTests(test::TestRunner& runner);
    void registerLimitTests(test::TestRunner& runner);
    void registerSaxTests(test::TestRunner& runner);
    void registerEncodingTests(test::TestRunner& runner);

    // ---- DTD и внешние ресурсы ----
    void registerDtdEntityTests(test::TestRunner& runner);
    void registerDtdValidationTests(test::TestRunner& runner);
    void registerExternalResourceTests(test::TestRunner& runner);

    // ---- Дополнительные модули ----
    void registerSerializerTests(test::TestRunner& runner);
    void registerTraversalTests(test::TestRunner& runner);
    void registerXPathPathTests(test::TestRunner& runner);
    void registerXPathFunctionTests(test::TestRunner& runner);

    // ---- XML Schema и XInclude ----
    void registerXsdDatatypeTests(test::TestRunner& runner);
    void registerXsdSchemaTests(test::TestRunner& runner);
    void registerXsdAdvancedTests(test::TestRunner& runner);
    void registerXIncludeTests(test::TestRunner& runner);

    // ---- Си-интерфейс для других языков ----
    void registerCapiTests(test::TestRunner& runner);
}

#endif
