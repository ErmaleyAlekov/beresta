#ifndef CONFORMANCE_CASE_RUNNER_HPP
#define CONFORMANCE_CASE_RUNNER_HPP

#include "conformance/test_case.hpp"

#include "core/document.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

namespace conformance
{
    /*
    ** Итог одной попытки разбора — «сырые факты» без оценки.
    ** Оценку (прошёл / не прошёл) выносит judgeCase() по типу случая:
    ** так разбор и правила вердикта не смешиваются (SRP).
    **
    ** The outcome of one parse attempt — "raw facts" without an assessment.
    ** The assessment (passed / failed) is made by judgeCase() by the case type:
    ** so parsing and the verdict rules are not mixed (SRP).
    */
    struct ParseAttempt
    {
        bool rejected = false;              // фатальная ошибка: документ не принят
                                            // a fatal error: the document is not accepted
        bool unexpected = false;            // брошено НЕ исключение библиотеки (сбой)
                                            // a NON-library exception was thrown (a failure)
        std::string fatalMessage;           // текст фатальной ошибки / the text of the fatal error
        std::size_t validityErrors = 0;     // число восстановимых ошибок (DTD)
                                            // the number of recoverable errors (DTD)
        std::string firstValidityError;     // первая из них — для отчёта / the first of them — for the report
        std::optional<beresta::Document> document;
    };

    /*
    ** CaseRunner — прогон одного случая (паттерн Template Method в функциональном
    ** виде: «применим ли случай -> разбор -> вердикт -> сравнение вывода»).
    ** Корень набора нужен резолверу: внешние DTD и сущности читаются только
    ** изнутри каталога xmlconf, наружу резолвер не выпустит.
    **
    ** CaseRunner — running one case (the Template Method pattern in a functional
    ** form: "is the case applicable -> parsing -> verdict -> output comparison").
    ** The suite root is needed by the resolver: external DTDs and entities are read only
    ** from inside the xmlconf directory, the resolver does not let them out.
    */
    class CaseRunner
    {
    public:
        explicit CaseRunner(std::filesystem::path suiteRoot);

        // Полный прогон случая с вердиктом
        // A full run of a case with a verdict
        CaseResult run(const TestCase& testCase) const;

    private:
        // Разбор документа с настройками, подходящими типу случая
        // Parses a document with settings suitable for the case type
        ParseAttempt attemptParse(const TestCase& testCase) const;

        std::filesystem::path _suiteRoot;
    };

    // Причина пропуска или пустая строка, если случай применим к XML 1.0 5-й редакции
    // The skip reason or an empty string if the case applies to XML 1.0 5th edition
    std::string skipReason(const TestCase& testCase);

    // Вердикт по фактам разбора (без сравнения канонического вывода)
    // The verdict by the parse facts (without comparing the canonical output)
    CaseResult judgeCase(const TestCase& testCase, const ParseAttempt& attempt);

    // Сравнение канонической формы с файлом OUTPUT; пусто — совпало
    // Compares the canonical form with the OUTPUT file; empty — it matched
    std::string compareOutput(const TestCase& testCase, const beresta::Document& document);
}

#endif
