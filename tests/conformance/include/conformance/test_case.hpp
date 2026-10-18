#ifndef CONFORMANCE_TEST_CASE_HPP
#define CONFORMANCE_TEST_CASE_HPP

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace conformance
{
    /*
    ** Категория случая из каталога W3C (атрибут TYPE элемента <TEST>):
    **  - VALID      — документ корректен и валиден по своему DTD;
    **  - INVALID    — документ well-formed, но нарушает ограничение валидности;
    **  - NOT_WF     — документ не well-formed: парсер ОБЯЗАН отвергнуть его;
    **  - ERROR_CASE — «ошибка» в смысле спецификации: процессор МОЖЕТ
    **                 сообщить о ней, а может и нет (результат информативный).
    **
    ** The case category from the W3C catalog (the TYPE attribute of the <TEST> element):
    **  - VALID      — the document is well-formed and valid by its DTD;
    **  - INVALID    — the document is well-formed but violates a validity constraint;
    **  - NOT_WF     — the document is not well-formed: the parser MUST reject it;
    **  - ERROR_CASE — an "error" in the sense of the specification: a processor MAY
    **                 report it or may not (the result is informative).
    */
    enum class CaseType
    {
        VALID,
        INVALID,
        NOT_WF,
        ERROR_CASE
    };

    /*
    ** Один случай набора тестов: всё, что нужно знать прогонщику.
    ** Пути уже абсолютные — xml:base всех предков учтён при чтении каталога.
    **
    ** One case of the test suite: everything the runner needs to know.
    ** The paths are already absolute — xml:base of all ancestors is applied when reading the catalog.
    */
    struct TestCase
    {
        std::string id;                     // уникальный ID случая (ID="...") / the unique case ID (ID="...")
        CaseType type = CaseType::VALID;    // ожидаемый результат / the expected result
        std::filesystem::path file;         // разбираемый документ (URI="...") / the document to parse (URI="...")
        std::filesystem::path output;       // канонический вывод (OUTPUT="...") или пусто
                                            // the canonical output (OUTPUT="...") or empty
        std::string location;               // путь документа относительно корня набора (для отчёта)
                                            // the document path relative to the suite root (for the report)
        std::string suite;                  // набор-источник (PROFILE ближайшего TESTCASES)
                                            // the source suite (PROFILE of the nearest TESTCASES)
        std::string entities;               // ENTITIES: none / parameter / general / both
        std::string recommendation;         // RECOMMENDATION: XML1.0, NS1.0, XML1.1...
        std::string edition;                // EDITION: "5" или "1 2 3 4" (пусто — любая)
                                            // EDITION: "5" or "1 2 3 4" (empty — any)
        std::string version;                // VERSION: версия XML документа (пусто — 1.0)
                                            // VERSION: the XML version of the document (empty — 1.0)
        std::string sections;               // разделы спецификации, которые проверяет случай
                                            // the specification sections the case tests
        bool namespaceAware = true;         // NAMESPACE="no" — без правил пространств имён
                                            // NAMESPACE="no" — without the namespace rules
    };

    // Вердикт по одному случаю
    // The verdict on one case
    enum class Verdict
    {
        PASSED,     // поведение парсера совпало с ожидаемым / the parser behavior matched the expected one
        FAILED,     // расхождение со стандартом / a deviation from the standard
        SKIPPED     // случай вне области применимости (XML 1.1, старые редакции)
                    // the case is out of scope (XML 1.1, old editions)
    };

    // Результат прогона одного случая
    // The result of running one case
    struct CaseResult
    {
        const TestCase* testCase = nullptr;
        Verdict verdict = Verdict::SKIPPED;
        std::string reason;                 // причина провала или пропуска / the reason of the failure or the skip
    };

    // Каталог целиком: случаи в порядке документа
    // The whole catalog: the cases in document order
    using Catalog = std::vector<TestCase>;
}

#endif
