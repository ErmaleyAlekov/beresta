#include "conformance/report.hpp"

#include <cstdio>

namespace conformance
{
    namespace
    {
        // Учитывает один вердикт в счётчиках группы
        // Accounts for one verdict in the group counters
        void countVerdict(Tally& tally, Verdict verdict)
        {
            if (verdict == Verdict::PASSED)
                ++tally.passed;
            else if (verdict == Verdict::FAILED)
                ++tally.failed;
            else
                ++tally.skipped;
        }
    }

    // Имена категорий совпадают со значениями атрибута TYPE каталога
    // The category names match the values of the TYPE attribute of the catalog
    std::string typeName(CaseType type)
    {
        if (type == CaseType::VALID)
            return "valid";
        if (type == CaseType::INVALID)
            return "invalid";
        if (type == CaseType::NOT_WF)
            return "not-wf";
        return "error";
    }

    // Каждый результат попадает в три счётчика: категория, набор, итог
    // Each result goes into three counters: the category, the suite, the total
    Summary summarize(const std::vector<CaseResult>& results)
    {
        Summary summary;

        for (const CaseResult& result : results)
        {
            countVerdict(summary.byType[typeName(result.testCase->type)], result.verdict);
            countVerdict(summary.bySuite[result.testCase->suite], result.verdict);
            countVerdict(summary.total, result.verdict);
        }
        return summary;
    }

    namespace detail
    {
        // Процент от применимых случаев; при отсутствии применимых — прочерк
        // The percentage of the applicable cases; a dash if there are none applicable
        std::string percent(const Tally& tally)
        {
            std::size_t applicable = tally.passed + tally.failed;
            char buffer[32];

            if (applicable == 0)
                return "—";
            std::snprintf(buffer, sizeof(buffer), "%.1f%%",
                100.0 * static_cast<double>(tally.passed) / static_cast<double>(applicable));
            return buffer;
        }
    }
}
