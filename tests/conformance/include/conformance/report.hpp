#ifndef CONFORMANCE_REPORT_HPP
#define CONFORMANCE_REPORT_HPP

#include "conformance/test_case.hpp"

#include <cstddef>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

namespace conformance
{
    // Счётчики одной группы (категории или набора)
    // The counters of one group (a category or a suite)
    struct Tally
    {
        std::size_t passed = 0;
        std::size_t failed = 0;
        std::size_t skipped = 0;
    };

    /*
    ** Сводка прогона: счётчики по категориям TYPE и по наборам-источникам.
    ** Процент считается от ПРИМЕНИМЫХ случаев (пропущенные не учитываются).
    **
    ** The run summary: counters by TYPE categories and by source suites.
    ** The percentage is computed from the APPLICABLE cases (skipped ones are not counted).
    */
    struct Summary
    {
        std::map<std::string, Tally> byType;
        std::map<std::string, Tally> bySuite;
        Tally total;
    };

    // Строит сводку по результатам
    // Builds the summary from the results
    Summary summarize(const std::vector<CaseResult>& results);

    // Человекочитаемое имя категории ("valid", "not-wf"...)
    // A human-readable category name ("valid", "not-wf"...)
    std::string typeName(CaseType type);

    // Краткая таблица в консоль
    // A short table to the console
    void printSummary(const Summary& summary, std::ostream& out);

    // Полный отчёт в Markdown: таблицы и список провалов с причинами
    // The full report in Markdown: tables and a list of failures with reasons
    void writeMarkdownReport(const Summary& summary, const std::vector<CaseResult>& results,
        std::ostream& out);

    namespace detail
    {
        // Процент прошедших от применимых, с одной цифрой после запятой
        // The percentage of passed among applicable, with one digit after the decimal point
        std::string percent(const Tally& tally);

        // Строка таблицы Markdown для группы
        // A Markdown table row for a group
        void writeTallyRow(const std::string& name, const Tally& tally, std::ostream& out);
    }
}

#endif
