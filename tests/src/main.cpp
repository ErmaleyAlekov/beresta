#include "framework/test_framework.hpp"
#include "suites/test_suites.hpp"

namespace
{
    // Функция регистрации одного набора тестов
    using SuiteRegistrar = void (*)(test::TestRunner&);

    // Все наборы в порядке запуска: новый набор достаточно добавить сюда
    const SuiteRegistrar SUITES[] = {
        suites::registerCopyTests, suites::registerHierarchyTests,
        suites::registerElementTests, suites::registerSerializeTests,
        suites::registerNavigationTests, suites::registerNamespaceTests,
        suites::registerFormatTests, suites::registerContentTests,
        suites::registerErrorTests, suites::registerLimitTests,
        suites::registerSaxTests, suites::registerEncodingTests,
        suites::registerDtdEntityTests, suites::registerDtdValidationTests,
        suites::registerExternalResourceTests,
        suites::registerSerializerTests, suites::registerTraversalTests, suites::registerXPathPathTests, suites::registerXPathFunctionTests,
        suites::registerXsdDatatypeTests, suites::registerXsdSchemaTests, suites::registerXsdAdvancedTests, suites::registerXIncludeTests,
        suites::registerCapiTests
    };
}

/*
** Точка входа тестов: регистрирует все наборы и запускает их.
** Код возврата 0 — всё прошло, 1 — есть упавшие тесты (для CI и ctest).
*/
int main()
{
    test::TestRunner runner;

    test::enableUtf8Console();
    for (SuiteRegistrar registrar : SUITES)
        registrar(runner);
    if (runner.runAll() != 0)
        return 1;
    return 0;
}
