#include "xsts/case_runner.hpp"
#include "xsts/catalog.hpp"
#include "xsts/console.hpp"
#include "xsts/limitations.hpp"
#include "xsts/report.hpp"
#include "xsts/xsts_error.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    // Аргументы командной строки прогонщика
    // The command line arguments of the runner
    struct Arguments
    {
        std::filesystem::path suiteRoot = "xmlschema2006-11-06";    // папка распакованного набора
                                                                    // the folder of the unpacked suite
        std::filesystem::path reportFile;                           // куда писать Markdown (пусто — не писать)
                                                                    // where to write the Markdown (empty — do not write)
        std::filesystem::path limitationsFile = "known_limitations.txt";
        std::string only;                                           // префикс "набор/группа" (отладка)
                                                                    // the "set/group" prefix (debugging)
        bool verbose = false;                                       // печатать группы и провалы
                                                                    // print the groups and the failures
    };

    /*
    ** Разбор аргументов:
    **   xsts_runner [папка_набора] [--report файл.md] [--limitations файл]
    **               [--only набор/группа] [--verbose]
    **
    ** Parsing the arguments:
    **   xsts_runner [suite_folder] [--report file.md] [--limitations file]
    **               [--only set/group] [--verbose]
    */
    Arguments parseArguments(int argc, char** argv)
    {
        Arguments arguments;

        for (int index = 1; index < argc; ++index)
        {
            std::string current = argv[index];
            bool hasValue = index + 1 < argc;

            if (current == "--report" && hasValue)
                arguments.reportFile = std::filesystem::u8path(argv[++index]);
            else if (current == "--limitations" && hasValue)
                arguments.limitationsFile = std::filesystem::u8path(argv[++index]);
            else if (current == "--only" && hasValue)
                arguments.only = argv[++index];
            else if (current == "--verbose")
                arguments.verbose = true;
            else if (!current.empty() && current[0] != '-')
                arguments.suiteRoot = std::filesystem::u8path(current);
            else
                throw xsts::XstsError("неизвестный аргумент: " + current);
        }
        return arguments;
    }

    // Печатает провалы группы при --verbose (сразу, чтобы видеть прогресс долгого прогона)
    // Prints the group failures with --verbose (at once, to see the progress of a long run)
    void printGroupFailures(const std::vector<xsts::CaseResult>& results, std::size_t from)
    {
        for (std::size_t index = from; index < results.size(); ++index)
        {
            if (results[index].verdict == xsts::Verdict::FAILED)
                std::cout << "  FAIL " << results[index].testCase->id << ": " << results[index].reason << "\n";
        }
    }

    // Прогоняет группы набора (или выбранные --only)
    // Runs the suite groups (or the ones chosen by --only)
    std::vector<xsts::CaseResult> runSuite(const xsts::Catalog& catalog, const Arguments& arguments,
        const xsts::GroupRunner& runner)
    {
        std::vector<xsts::CaseResult> results;

        for (const xsts::TestGroup& group : catalog.groups)
        {
            std::string id = group.testSet + "/" + group.name;
            std::size_t from = results.size();

            if (id.compare(0, arguments.only.size(), arguments.only) != 0)
                continue;
            if (arguments.verbose)
                std::cout << "  " << id << std::endl;
            runner.run(catalog, group, results);
            if (arguments.verbose)
                printGroupFailures(results, from);
        }
        return results;
    }

    // Записывает Markdown-отчёт, если он запрошен
    // Writes the Markdown report if it is requested
    void saveReport(const xsts::Summary& summary, const std::vector<xsts::CaseResult>& results,
        const std::filesystem::path& file)
    {
        if (file.empty())
            return;
        std::ofstream stream(file, std::ios::binary);

        if (!stream)
            throw xsts::XstsError("не удалось открыть файл отчёта " + file.u8string());
        xsts::writeMarkdownReport(summary, results, stream);
        std::cout << "\n  Отчёт: " << file.u8string() << "\n";
    }
}

/*
** Прогонщик W3C XML Schema Test Suite.
** Код возврата: 0 — прогон выполнен (расхождения описаны в отчёте),
** 2 — сбой инфраструктуры (нет набора, неверные аргументы).
**
** The W3C XML Schema Test Suite runner.
** The exit code: 0 — the run is done (the deviations are described in the report),
** 2 — an infrastructure failure (no suite, invalid arguments).
*/
int main(int argc, char** argv)
{
    xsts::enableUtf8Console();
    try
    {
        Arguments arguments = parseArguments(argc, argv);
        std::filesystem::path root = std::filesystem::absolute(arguments.suiteRoot).lexically_normal();
        xsts::Catalog catalog = xsts::loadSuite(root);
        xsts::Limitations limitations = xsts::Limitations::load(arguments.limitationsFile);
        std::vector<xsts::CaseResult> results = runSuite(catalog, arguments, xsts::GroupRunner(root, limitations));
        xsts::Summary summary = xsts::summarize(results);

        std::cout << "  Случаев в наборе: " << catalog.cases.size() << ", групп: " << catalog.groups.size() << "\n";
        xsts::printSummary(summary, std::cout);
        saveReport(summary, results, arguments.reportFile);
    }
    catch (const xsts::XstsError& problem)
    {
        std::cerr << "xsts: " << problem.what() << "\n";
        return 2;
    }
    return 0;
}
