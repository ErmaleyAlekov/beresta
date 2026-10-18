#include "conformance/report.hpp"

#include <map>
#include <ostream>

namespace conformance
{
    namespace
    {
        // Таблица счётчиков: заголовок, строки групп и итоговая строка
        // The counter table: the header, the group rows and the total row
        void writeTallyTable(const std::string& title, const std::map<std::string, Tally>& groups,
            const Tally& total, std::ostream& out)
        {
            out << "\n## " << title << "\n\n";
            out << "| Группа | Прошло | Провал | Пропуск | Процент |\n";
            out << "|---|---:|---:|---:|---:|\n";
            for (const auto& entry : groups)
                detail::writeTallyRow(entry.first, entry.second, out);
            detail::writeTallyRow("**Итого**", total, out);
        }

        // Список провалов одной категории: ID, файл, разделы спецификации, причина
        // The list of failures of one category: the ID, the file, the specification sections, the reason
        void writeFailures(CaseType type, const std::vector<CaseResult>& results, std::ostream& out)
        {
            std::size_t count = 0;

            out << "\n### " << typeName(type) << "\n\n";
            for (const CaseResult& result : results)
            {
                const TestCase& testCase = *result.testCase;

                if (result.verdict != Verdict::FAILED || testCase.type != type)
                    continue;
                out << "- `" << testCase.id << "` (" << testCase.location << ", разд. "
                    << testCase.sections << ") — " << result.reason << "\n";
                ++count;
            }
            if (count == 0)
                out << "Провалов нет.\n";
        }

        // Сводка причин пропуска: сколько случаев и почему не применимы
        // The summary of the skip reasons: how many cases and why they are not applicable
        void writeSkipReasons(const std::vector<CaseResult>& results, std::ostream& out)
        {
            std::map<std::string, std::size_t> reasons;

            for (const CaseResult& result : results)
            {
                if (result.verdict == Verdict::SKIPPED)
                    ++reasons[result.reason];
            }
            out << "\n## Пропущенные случаи\n\n";
            for (const auto& entry : reasons)
                out << "- " << entry.first << ": " << entry.second << "\n";
        }
    }

    /*
    ** Структура отчёта: итог по категориям TYPE, итог по наборам-источникам,
    ** причины пропусков, затем все провалы, сгруппированные по категориям.
    **
    ** The report structure: the totals by TYPE categories, the totals by source suites,
    ** the skip reasons, then all failures grouped by categories.
    */
    void writeMarkdownReport(const Summary& summary, const std::vector<CaseResult>& results,
        std::ostream& out)
    {
        const CaseType types[] = {CaseType::VALID, CaseType::INVALID, CaseType::NOT_WF,
            CaseType::ERROR_CASE};

        out << "# Берёста: соответствие W3C XML Conformance Test Suite\n\n";
        out << "Набор: xmlts20130923 (W3C/OASIS). Процент считается от применимых случаев.\n";
        out << "Отчёт создан прогонщиком tests/conformance (make run).\n";
        writeTallyTable("По категориям", summary.byType, summary.total, out);
        writeTallyTable("По наборам", summary.bySuite, summary.total, out);
        writeSkipReasons(results, out);
        out << "\n## Провалы\n";
        for (CaseType type : types)
            writeFailures(type, results, out);
    }
}
