#include "conformance/report.hpp"

#include <iomanip>
#include <ostream>

namespace conformance
{
    namespace
    {
        // Одна строка консольной таблицы: имя, прошло, провалено, пропущено, процент
        // One row of the console table: the name, passed, failed, skipped, the percentage
        void printTallyLine(const std::string& name, const Tally& tally, std::ostream& out)
        {
            out << "  " << std::left << std::setw(10) << name << std::right
                << std::setw(7) << tally.passed
                << std::setw(9) << tally.failed
                << std::setw(10) << tally.skipped
                << "   " << detail::percent(tally) << "\n";
        }
    }

    // Консольная сводка: только категории и итог (наборы — в Markdown-отчёте)
    // The console summary: only the categories and the total (the suites are in the Markdown report)
    void printSummary(const Summary& summary, std::ostream& out)
    {
        out << "\n  Категория  Прошло  Провал  Пропуск   Процент\n";
        for (const auto& entry : summary.byType)
            printTallyLine(entry.first, entry.second, out);
        printTallyLine("ИТОГО", summary.total, out);
    }

    namespace detail
    {
        // Строка Markdown-таблицы с теми же колонками, что и в консоли
        // A Markdown table row with the same columns as in the console
        void writeTallyRow(const std::string& name, const Tally& tally, std::ostream& out)
        {
            out << "| " << name << " | " << tally.passed << " | " << tally.failed
                << " | " << tally.skipped << " | " << percent(tally) << " |\n";
        }
    }
}
