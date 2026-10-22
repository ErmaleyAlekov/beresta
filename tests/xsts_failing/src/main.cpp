#include "failing/case_executor.hpp"
#include "failing/debug_run.hpp"
#include "failing/failing_list.hpp"
#include "failing/options.hpp"
#include "failing/regenerate.hpp"

#include "xsts/catalog.hpp"
#include "xsts/console.hpp"
#include "xsts/limitations.hpp"
#include "xsts/xsts_error.hpp"

#include <filesystem>
#include <iostream>
#include <vector>

namespace
{
    /*
    ** Прогон случаев из списка. Код возврата: 0 — все выбранные случаи
    ** уже проходят, 1 — хотя бы один всё ещё проваливается.
    **
    ** Running the cases from the list. The exit code: 0 — all the selected cases
    ** already pass, 1 — at least one still fails.
    */
    int runDebug(const failing::SuiteView& suite, const failing::Options& options)
    {
        std::vector<failing::FailingEntry> entries = failing::readFailingList(options.listFile);
        failing::RunTotals totals = failing::runListed(entries, suite, options.run, std::cout);

        std::cout << "\n  Прошло " << totals.passed << " из " << totals.selected
                  << " (всего в списке " << entries.size() << ")\n";
        if (totals.selected == 0)
            std::cout << "  Под фильтр не попал ни один случай\n";
        if (totals.passed > 0)
            std::cout << "  Исправленные случаи уйдут из списка после make regen\n";
        if (totals.passed == totals.selected)
            return 0;
        return 1;
    }

    // Полный прогон набора и запись нового списка провалов со сводкой
    // A full suite run and writing a new failure list with a summary
    int regenerateList(const failing::SuiteView& suite, const failing::Options& options)
    {
        xsts::Limitations limitations = xsts::Limitations::load(options.limitationsFile);
        std::vector<failing::FailingEntry> entries = failing::collectFailing(suite, limitations, std::cout);

        failing::writeFailingList(options.listFile, entries);
        std::cout << "\n  Провалов: " << entries.size() << ", список: " << options.listFile.u8string() << "\n\n";
        failing::printBreakdown(entries, std::cout);
        return 0;
    }
}

/*
** Отладчик провалов W3C XML Schema Test Suite: выполняет на библиотеке
** только случаи из failing_cases.tsv (или пересобирает этот список).
** Код возврата 2 — сбой окружения (нет набора или списка, неверные аргументы).
**
** The W3C XML Schema Test Suite failure debugger: runs on the library
** only the cases from failing_cases.tsv (or rebuilds this list).
** Exit code 2 — an environment failure (no suite or list, invalid arguments).
*/
int main(int argc, char** argv)
{
    xsts::enableUtf8Console();
    try
    {
        failing::Options options = failing::parseOptions(argc, argv);
        std::filesystem::path root = std::filesystem::absolute(options.suiteRoot).lexically_normal();
        xsts::Catalog catalog = xsts::loadSuite(root);
        failing::CaseIndex index = failing::indexCatalog(catalog);
        failing::SuiteView suite{catalog, index, root};

        if (options.regenerate)
            return regenerateList(suite, options);
        return runDebug(suite, options);
    }
    catch (const xsts::XstsError& problem)
    {
        std::cerr << "xsts_failing: " << problem.what() << "\n";
        return 2;
    }
}
