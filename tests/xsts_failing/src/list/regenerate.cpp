#include "failing/regenerate.hpp"
#include "failing/failing_list.hpp"

#include "xsts/case_runner.hpp"

#include <ostream>
#include <string>

namespace failing
{
    namespace
    {
        // Вид случая словом из метаданных набора
        // The case kind as the word from the suite metadata
        std::string kindWord(const xsts::TestCase& testCase)
        {
            if (testCase.kind == xsts::CaseKind::SCHEMA)
                return "schemaTest";
            return "instanceTest";
        }

        // Ожидание словом из метаданных набора (<expected validity="..."/>)
        // The expectation as the word from the suite metadata (<expected validity="..."/>)
        std::string expectationWord(const xsts::TestCase& testCase)
        {
            if (testCase.expected == xsts::Expectation::VALID)
                return "valid";
            return "invalid";
        }
    }

    /*
    ** Группы прогоняются в порядке каталога; имя набора печатается при
    ** смене набора — полный прогон идёт минуты, прогресс должен быть виден.
    **
    ** The groups are run in the catalog order; the set name is printed when
    ** the set changes — a full run takes minutes, the progress must be visible.
    */
    std::vector<FailingEntry> collectFailing(const SuiteView& suite, const xsts::Limitations& limitations,
        std::ostream& progress)
    {
        xsts::GroupRunner runner(suite.root, limitations);
        std::vector<FailingEntry> entries;
        std::string currentSet;

        for (const xsts::TestGroup& group : suite.catalog.groups)
        {
            std::vector<xsts::CaseResult> results;

            if (group.testSet != currentSet)
            {
                currentSet = group.testSet;
                progress << "  " << currentSet << std::endl;
            }
            runner.run(suite.catalog, group, results);
            for (const xsts::CaseResult& result : results)
            {
                if (result.verdict == xsts::Verdict::FAILED)
                    entries.push_back(detail::makeEntry(suite, result));
            }
        }
        return entries;
    }

    namespace detail
    {
        /*
        ** Сообщение — причина из прогонщика (как в xsts_report.md), факт —
        ** из отдельного выполнения случая. Указатель testCase смотрит в
        ** Catalog::cases, поэтому разность указателей — индекс случая.
        **
        ** The message is the reason from the runner (as in xsts_report.md), the fact is
        ** from a separate run of the case. The testCase pointer points into
        ** Catalog::cases, so the pointer difference is the case index.
        */
        FailingEntry makeEntry(const SuiteView& suite, const xsts::CaseResult& result)
        {
            const xsts::TestCase& testCase = *result.testCase;
            std::size_t position = static_cast<std::size_t>(result.testCase - suite.catalog.cases.data());
            const xsts::TestGroup& group = suite.catalog.groups[suite.index.groupOf[position]];
            FailingEntry entry;

            entry.id = testCase.id;
            entry.kind = kindWord(testCase);
            entry.expected = expectationWord(testCase);
            entry.actual = executeCase(suite, position).actual;
            entry.documents = joinDocuments(testCase.documents, suite.root);
            entry.schema = "-";
            if (testCase.kind == xsts::CaseKind::INSTANCE && group.hasSchema)
                entry.schema = joinDocuments(suite.catalog.cases[group.schemaCase].documents, suite.root);
            entry.message = result.reason;
            entry.description = testCase.description;
            return entry;
        }
    }
}
