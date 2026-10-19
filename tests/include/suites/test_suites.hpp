#ifndef TEST_SUITES_HPP
#define TEST_SUITES_HPP

#include "framework/test_framework.hpp"

namespace suites
{
    // Каждая функция регистрирует в раннере тесты своего набора
    // Each function registers the tests of its suite in the runner

    // ---- Ядро ----
    // ---- Core ----
    void registerCopyTests(test::TestRunner& runner);
    void registerHierarchyTests(test::TestRunner& runner);
    void registerElementTests(test::TestRunner& runner);
    void registerSerializeTests(test::TestRunner& runner);
    void registerNavigationTests(test::TestRunner& runner);
    void registerNamespaceTests(test::TestRunner& runner);
    void registerDeepTreeTests(test::TestRunner& runner);

    // ---- Парсер ----
    // ---- Parser ----
    void registerFormatTests(test::TestRunner& runner);
    void registerContentTests(test::TestRunner& runner);
    void registerErrorTests(test::TestRunner& runner);
    void registerLimitTests(test::TestRunner& runner);
    void registerFloodTests(test::TestRunner& runner);
    void registerAttributeScaleTests(test::TestRunner& runner);
    void registerMappedFileTests(test::TestRunner& runner);
    void registerSaxTests(test::TestRunner& runner);
    void registerSaxDtdTests(test::TestRunner& runner);
    void registerEncodingTests(test::TestRunner& runner);

    // ---- DTD и внешние ресурсы ----
    // ---- DTD and external resources ----
    void registerDtdEntityTests(test::TestRunner& runner);
    void registerDtdValidationTests(test::TestRunner& runner);
    void registerExternalResourceTests(test::TestRunner& runner);

    // ---- Расхождения с W3C XML Conformance Test Suite (tests/conformance) ----
    // ---- Deviations from the W3C XML Conformance Test Suite (tests/conformance) ----
    void registerW3cSyntaxTests(test::TestRunner& runner);
    void registerW3cEntityTests(test::TestRunner& runner);
    void registerW3cValidityTests(test::TestRunner& runner);
    void registerW3cEncodingTests(test::TestRunner& runner);

    // ---- Находки фаззинга (tests/fuzz) ----
    // ---- Fuzzing findings (tests/fuzz) ----
    void registerFuzzRegressionTests(test::TestRunner& runner);

    // ---- Дополнительные модули ----
    // ---- Additional modules ----
    void registerSerializerTests(test::TestRunner& runner);
    void registerTraversalTests(test::TestRunner& runner);
    void registerXPathPathTests(test::TestRunner& runner);
    void registerXPathFunctionTests(test::TestRunner& runner);
    void registerXPathNamespaceTests(test::TestRunner& runner);
    void registerXPathNameTests(test::TestRunner& runner);

    // ---- XML Schema и XInclude ----
    // ---- XML Schema and XInclude ----
    void registerXsdDatatypeTests(test::TestRunner& runner);
    void registerXsdSchemaTests(test::TestRunner& runner);
    void registerXsdAdvancedTests(test::TestRunner& runner);
    void registerXsdIdentityTests(test::TestRunner& runner);
    void registerXsdIdentityScopeTests(test::TestRunner& runner);
    void registerXsdPathTests(test::TestRunner& runner);
    void registerXsdStreamTests(test::TestRunner& runner);
    void registerXsdStreamIdentityTests(test::TestRunner& runner);
    void registerXsdSubstitutionTests(test::TestRunner& runner);
    void registerXsdDerivationTests(test::TestRunner& runner);
    void registerXsdFinalTests(test::TestRunner& runner);
    void registerXsdRedefineTests(test::TestRunner& runner);
    void registerXsdDeepTests(test::TestRunner& runner);
    void registerContentCursorTests(test::TestRunner& runner);
    void registerContentPlanTests(test::TestRunner& runner);
    void registerXIncludeTests(test::TestRunner& runner);

    // ---- Си-интерфейс для других языков ----
    // ---- The C interface for other languages ----
    void registerCapiTests(test::TestRunner& runner);
    void registerCapiMoveTests(test::TestRunner& runner);
    void registerCapiStreamTests(test::TestRunner& runner);
    void registerCapiParallelTests(test::TestRunner& runner);
    void registerCapiBulkMoveTests(test::TestRunner& runner);

    // ---- Языки сообщений / Message languages ----
    void registerI18nTests(test::TestRunner& runner);
}

#endif
