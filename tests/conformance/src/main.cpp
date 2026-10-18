#include "conformance/case_runner.hpp"
#include "conformance/catalog.hpp"
#include "conformance/conformance_error.hpp"
#include "conformance/console.hpp"
#include "conformance/report.hpp"

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
        std::filesystem::path suiteRoot = "xmlconf";        // папка распакованного набора
                                                            // the folder of the unpacked suite
        std::filesystem::path reportFile;                   // куда писать Markdown (пусто — не писать)
                                                            // where to write the Markdown (empty — do not write)
        std::string onlyId;                                 // прогнать один случай (отладка падений)
                                                            // run one case (for debugging crashes)
        bool verbose = false;                               // печатать каждый провал в консоль
                                                            // print every failure to the console
    };

    /*
    ** Разбор аргументов:
    **   conformance_runner [папка_xmlconf] [--report файл.md] [--only ID] [--verbose]
    **
    ** Parsing the arguments:
    **   conformance_runner [xmlconf_folder] [--report file.md] [--only ID] [--verbose]
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
            else if (current == "--only" && hasValue)
                arguments.onlyId = argv[++index];
            else if (current == "--verbose")
                arguments.verbose = true;
            else if (!current.empty() && current[0] != '-')
                arguments.suiteRoot = std::filesystem::u8path(current);
            else
                throw conformance::ConformanceError("неизвестный аргумент: " + current);
        }
        return arguments;
    }

    // Прогоняет случаи каталога (или один выбранный); каждый провал печатается при --verbose
    // Runs the catalog cases (or one chosen case); each failure is printed with --verbose
    std::vector<conformance::CaseResult> runCatalog(const conformance::Catalog& catalog,
        const Arguments& arguments, const std::filesystem::path& root)
    {
        conformance::CaseRunner runner(root);
        std::vector<conformance::CaseResult> results;

        for (const conformance::TestCase& testCase : catalog)
        {
            if (!arguments.onlyId.empty() && testCase.id != arguments.onlyId)
                continue;
            if (arguments.verbose)
                std::cout << "  " << testCase.id << std::flush;
            results.push_back(runner.run(testCase));
            if (arguments.verbose)
                std::cout << "  [" << results.back().reason << "]\n";
        }
        return results;
    }

    // Записывает Markdown-отчёт, если он запрошен
    // Writes the Markdown report if it is requested
    void saveReport(const conformance::Summary& summary,
        const std::vector<conformance::CaseResult>& results, const std::filesystem::path& file)
    {
        if (file.empty())
            return;
        std::ofstream stream(file, std::ios::binary);

        if (!stream)
            throw conformance::ConformanceError("не удалось открыть файл отчёта " + file.u8string());
        conformance::writeMarkdownReport(summary, results, stream);
        std::cout << "\n  Отчёт: " << file.u8string() << "\n";
    }
}

/*
** Прогонщик W3C XML Conformance Test Suite.
** Код возврата: 0 — прогон выполнен (провалы описаны в отчёте),
** 2 — сбой инфраструктуры (нет набора, неверные аргументы).
**
** The W3C XML Conformance Test Suite runner.
** The exit code: 0 — the run is done (the failures are described in the report),
** 2 — an infrastructure failure (no suite, invalid arguments).
*/
int main(int argc, char** argv)
{
    conformance::enableUtf8Console();
    try
    {
        Arguments arguments = parseArguments(argc, argv);
        std::filesystem::path root = std::filesystem::absolute(arguments.suiteRoot).lexically_normal();
        conformance::Catalog catalog = conformance::loadCatalog(root / "xmlconf.xml");
        std::vector<conformance::CaseResult> results = runCatalog(catalog, arguments, root);
        conformance::Summary summary = conformance::summarize(results);

        std::cout << "  Случаев в каталоге: " << catalog.size() << "\n";
        conformance::printSummary(summary, std::cout);
        saveReport(summary, results, arguments.reportFile);
    }
    catch (const conformance::ConformanceError& problem)
    {
        std::cerr << "conformance: " << problem.what() << "\n";
        return 2;
    }
    return 0;
}
