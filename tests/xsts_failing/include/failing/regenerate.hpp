#ifndef FAILING_REGENERATE_HPP
#define FAILING_REGENERATE_HPP

#include "failing/case_executor.hpp"
#include "failing/failing_case.hpp"

#include "xsts/limitations.hpp"
#include "xsts/test_case.hpp"

#include <iosfwd>
#include <vector>

namespace failing
{
    /*
    ** Пересобирает список провалов: весь набор прогоняется ТЕМ ЖЕ
    ** xsts::GroupRunner, что строит xsts_report.md (с known_limitations.txt
    ** и пропусками), поэтому в список попадают ровно провалы отчёта.
    ** Затем каждый провал выполняется отдельно (executeCase), чтобы
    ** записать фактическое поведение библиотеки.
    **
    ** Rebuilds the failure list: the whole suite is run by THE SAME
    ** xsts::GroupRunner that builds xsts_report.md (with known_limitations.txt
    ** and skips), so the list gets exactly the report failures.
    ** Then each failure is run separately (executeCase) to record
    ** the actual behavior of the library.
    */
    std::vector<FailingEntry> collectFailing(const SuiteView& suite, const xsts::Limitations& limitations,
        std::ostream& progress);

    // Сводка списка по наборам и по видам случаев — для README
    // A summary of the list by sets and by case kinds — for the README
    void printBreakdown(const std::vector<FailingEntry>& entries, std::ostream& out);

    namespace detail
    {
        // Запись списка по провалу прогонщика
        // A list entry for a runner failure
        FailingEntry makeEntry(const SuiteView& suite, const xsts::CaseResult& result);
    }
}

#endif
