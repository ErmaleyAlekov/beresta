#ifndef XSTS_REPORT_HPP
#define XSTS_REPORT_HPP

#include "xsts/test_case.hpp"

#include <cstddef>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

namespace xsts
{
    // Счётчики одной группы отчёта (категории, набора, каталога, возможности)
    // The counters of one report group (a category, a set, a directory, a feature)
    struct Tally
    {
        std::size_t passed = 0;
        std::size_t failed = 0;
        std::size_t skipped = 0;
        std::size_t limited = 0;
    };

    /*
    ** Сводка прогона. Процент считается от ПРИМЕНИМЫХ случаев: пропущенные
    ** и упёршиеся в сознательные ограничения в знаменатель не входят.
    **
    ** The run summary. The percentage is computed from the APPLICABLE cases: the skipped
    ** ones and the ones hitting deliberate limitations are not in the denominator.
    */
    struct Summary
    {
        std::map<std::string, Tally> byKind;        // "schemaTest valid" и т.п. / and so on
        std::map<std::string, Tally> byTestSet;     // имя testSet / the testSet name
        std::map<std::string, Tally> byDirectory;   // каталог данных / the data directory
        std::map<std::string, Tally> byFeature;     // метка возможности / the feature label
        Tally total;
    };

    // Строит сводку по результатам
    // Builds the summary from the results
    Summary summarize(const std::vector<CaseResult>& results);

    // Имя категории: "schemaTest valid", "instanceTest invalid"...
    // The category name: "schemaTest valid", "instanceTest invalid"...
    std::string kindName(const TestCase& testCase);

    // Краткая таблица в консоль
    // A short table to the console
    void printSummary(const Summary& summary, std::ostream& out);

    // Полный отчёт в Markdown: таблицы, пропуски, ограничения и расхождения
    // The full report in Markdown: tables, skips, limitations and deviations
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

        // Таблица счётчиков: заголовок, строки групп и итоговая строка
        // The counter table: the header, the group rows and the total row
        void writeTallyTable(const std::string& title, const std::map<std::string, Tally>& groups,
            const Tally* total, std::ostream& out);

        // Расхождения, сгруппированные по наборам
        // The deviations grouped by sets
        void writeFailures(const std::vector<CaseResult>& results, std::ostream& out);

        // Случаи, упёршиеся в ограничения, — сводка по причинам и список
        // The cases hitting limitations — a summary by reasons and a list
        void writeLimited(const std::vector<CaseResult>& results, std::ostream& out);
    }
}

#endif
