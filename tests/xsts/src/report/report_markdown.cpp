#include "xsts/report.hpp"

#include <map>
#include <ostream>

namespace xsts
{
    namespace
    {
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
            if (reasons.empty())
                out << "Случаев нет.\n";
            for (const auto& entry : reasons)
                out << "- " << entry.first << ": " << entry.second << "\n";
        }
    }

    namespace detail
    {
        // Строка таблицы Markdown: имя, счётчики и процент от применимых
        // A Markdown table row: the name, the counters and the percentage of the applicable ones
        void writeTallyRow(const std::string& name, const Tally& tally, std::ostream& out)
        {
            out << "| " << name << " | " << tally.passed << " | " << tally.failed << " | "
                << tally.limited << " | " << tally.skipped << " | " << percent(tally) << " |\n";
        }

        // total == nullptr — таблица без итоговой строки (метки возможностей пересекаются)
        // total == nullptr — a table without the total row (the feature labels overlap)
        void writeTallyTable(const std::string& title, const std::map<std::string, Tally>& groups,
            const Tally* total, std::ostream& out)
        {
            out << "\n## " << title << "\n\n";
            out << "| Группа | Прошло | Провал | Ограничение | Пропуск | Процент |\n";
            out << "|---|---:|---:|---:|---:|---:|\n";
            for (const auto& entry : groups)
                writeTallyRow(entry.first, entry.second, out);
            if (total != nullptr)
                writeTallyRow("**Итого**", *total, out);
        }
    }

    /*
    ** Структура отчёта: категории (вид случая + ожидание), наборы,
    ** каталоги данных, возможности, пропуски, ограничения и расхождения.
    **
    ** The report structure: the categories (case kind + expectation), the sets,
    ** the data directories, the features, the skips, the limitations and the deviations.
    */
    void writeMarkdownReport(const Summary& summary, const std::vector<CaseResult>& results,
        std::ostream& out)
    {
        out << "# Берёста: соответствие W3C XML Schema Test Suite\n\n";
        out << "Набор: xsts-2007-06-20 (XML Schema 1.0, 2-я редакция). Процент считается от\n";
        out << "применимых случаев: пропущенные и упёршиеся в сознательные ограничения\n";
        out << "в знаменатель не входят. Отчёт создан прогонщиком tests/xsts (make run).\n";
        detail::writeTallyTable("По категориям", summary.byKind, &summary.total, out);
        detail::writeTallyTable("По наборам", summary.byTestSet, &summary.total, out);
        detail::writeTallyTable("По каталогам данных", summary.byDirectory, &summary.total, out);
        detail::writeTallyTable("По возможностям (метки по тексту документов, пересекаются)",
            summary.byFeature, nullptr, out);
        writeSkipReasons(results, out);
        detail::writeLimited(results, out);
        detail::writeFailures(results, out);
    }
}
