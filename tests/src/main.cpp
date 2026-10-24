#include "framework/test_framework.hpp"
#include "suites/test_suites.hpp"
#include "i18n/i18n.hpp"

namespace
{
    // Функция регистрации одного набора тестов
    // The registration function of one test suite
    using SuiteRegistrar = void (*)(test::TestRunner&);

    // Все наборы в порядке запуска: новый набор достаточно добавить сюда
    // All suites in the run order: a new suite only needs to be added here
    const SuiteRegistrar SUITES[] = {
        suites::registerCopyTests, suites::registerHierarchyTests,
        suites::registerElementTests, suites::registerSerializeTests,
        suites::registerNavigationTests, suites::registerNamespaceTests, suites::registerDeepTreeTests,
        suites::registerFormatTests, suites::registerContentTests,
        suites::registerErrorTests, suites::registerLimitTests, suites::registerFloodTests,
        suites::registerAttributeScaleTests, suites::registerMappedFileTests,
        suites::registerSaxTests, suites::registerSaxDtdTests, suites::registerEncodingTests,
        suites::registerDtdEntityTests, suites::registerDtdValidationTests,
        suites::registerExternalResourceTests,
        suites::registerW3cSyntaxTests, suites::registerW3cEntityTests,
        suites::registerW3cValidityTests, suites::registerW3cEncodingTests,
        suites::registerFuzzRegressionTests,
        suites::registerSerializerTests, suites::registerTraversalTests, suites::registerXPathPathTests, suites::registerXPathFunctionTests, suites::registerXPathNamespaceTests,
        suites::registerXPathNameTests, suites::registerXPathDepthTests, suites::registerDecimalTests,
        suites::registerXsdDatatypeTests, suites::registerXsdSchemaTests, suites::registerXsdAdvancedTests, suites::registerXsdIdentityTests, suites::registerXsdIdentityScopeTests, suites::registerXsdPathTests,
        suites::registerXsdStreamTests, suites::registerXsdStreamIdentityTests,
        suites::registerXsdSubstitutionTests, suites::registerXsdDerivationTests, suites::registerXsdFinalTests,
        suites::registerXsdRedefineTests, suites::registerXsdSchemaLexicalTests,
        suites::registerXsdRedefineRulesTests, suites::registerXsdXsiTypeRootTests, suites::registerXsdDeepTests, suites::registerXsdDeepSchemaTests,
        suites::registerXsdDurationOrderTests, suites::registerXsdAttributeWildcardTests,
        suites::registerXsdValueConstraintTests, suites::registerXsdIdentityFieldTests,
        suites::registerXsdSchemaRulesTests, suites::registerXsdLeafDeclarationTests,
        suites::registerXsdAssertTests, suites::registerXsdAssertRulesTests,
        suites::registerXsdAssertXPathTests, suites::registerXsdAssertSchemaTests,
        suites::registerXsdAssertParallelTests, suites::registerXsdRegexUnicodeTests, suites::registerXsdRegexSyntaxTests, suites::registerXsdRegexTableTests,
        suites::registerContentCursorTests, suites::registerContentPlanTests, suites::registerXIncludeTests,
        suites::registerCapiTests, suites::registerCapiMoveTests, suites::registerCapiStreamTests,
        suites::registerCapiParallelTests, suites::registerCapiBulkMoveTests, suites::registerI18nTests
    };
}

/*
** Точка входа тестов: регистрирует все наборы и запускает их.
** Код возврата 0 — всё прошло, 1 — есть упавшие тесты (для CI и ctest).
**
** The test entry point: registers all suites and runs them.
** Exit code 0 — everything passed, 1 — there are failed tests (for CI and ctest).
*/
int main()
{
    test::TestRunner runner;

    test::enableUtf8Console();
    // Тексты ошибок в тестах сверяются по-русски — независимо от ERRORS_LANG сборки
    // Tests compare error texts in Russian — regardless of the build ERRORS_LANG
    beresta::i18n::setLanguage("ru");
    for (SuiteRegistrar registrar : SUITES)
        registrar(runner);
    if (runner.runAll() != 0)
        return 1;
    return 0;
}
