#include "bench/engine.hpp"
#include "bench/measure.hpp"
#include "bench/options.hpp"
#include "bench/report.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/*
** Программа замеров «Берёсты» и других XML-библиотек на одинаковых данных.
**   beresta_bench [--runs N] [--items N] [--paragraphs N] [--report файл.md]
** Служебный режим (запускается самой программой для замера памяти):
**   beresta_bench --memory <номер библиотеки> <catalog|article> [--items N] [--paragraphs N]
**
** A benchmark of Beresta and other XML libraries on the same data.
**   beresta_bench [--runs N] [--items N] [--paragraphs N] [--report file.md]
** A service mode (the program runs it itself to measure memory):
**   beresta_bench --memory <library number> <catalog|article> [--items N] [--paragraphs N]
*/

namespace
{
    // Наборы данных по имени (пустое имя — все) / The data sets by name (an empty name — all)
    std::vector<bench::Workload> makeWorkloads(const bench::Options& options, const std::string& only)
    {
        std::vector<bench::Workload> workloads;

        if (only.empty() || only == "catalog")
            workloads.push_back(bench::makeCatalog(options.items));
        if (only.empty() || only == "article")
            workloads.push_back(bench::makeArticle(options.paragraphs));
        return workloads;
    }

    // Служебный режим: прирост резидентной памяти после разбора / The service mode: the resident memory growth after parsing
    int runMemoryMode(const bench::Options& options)
    {
        std::vector<std::unique_ptr<bench::XmlEngine>> engines = bench::createEngines();
        std::vector<bench::Workload> workloads = makeWorkloads(options, options.memoryWorkload);

        if (static_cast<std::size_t>(options.memoryEngine) >= engines.size() || workloads.empty())
            return 1;
        double before = bench::residentBytes();
        engines[static_cast<std::size_t>(options.memoryEngine)]->parse(workloads[0].xml);
        std::cout << static_cast<long long>(bench::residentBytes() - before) << '\n';
        return 0;
    }

    // Все операции одной библиотеки и память в отдельном процессе / All operations of one library and the memory in a separate process
    bench::EngineResult runEngine(bench::XmlEngine& engine, std::size_t index, const bench::Workload& workload,
        const bench::Options& options, const std::string& program)
    {
        bench::EngineResult result{engine.name(), {}, 0.0};
        std::string arguments = workload.name + " --items " + std::to_string(options.items)
            + " --paragraphs " + std::to_string(options.paragraphs);

        for (std::size_t operation = 0; operation < bench::OPERATION_COUNT; ++operation)
            result.operations[operation] = bench::measure(engine, static_cast<bench::Operation>(operation),
                workload, options.runs);
        engine.release();
        result.memoryBytes = bench::measureMemoryInChild(program, std::to_string(index), arguments);
        std::cerr << "  " << result.engine << ": готово / done\n";
        return result;
    }

    // Один набор данных: все библиотеки, таблицы в stdout и в отчёт / One data set: all libraries, tables to stdout and the report
    void runWorkload(std::vector<std::unique_ptr<bench::XmlEngine>>& engines, const bench::Workload& workload,
        const bench::Options& options, const std::string& program, std::ofstream& report)
    {
        std::vector<bench::EngineResult> results;

        std::cerr << workload.name << " (" << workload.xml.size() / 1000000 << " MB)\n";
        for (std::size_t index = 0; index < engines.size(); ++index)
            results.push_back(runEngine(*engines[index], index, workload, options, program));
        bench::printTable(std::cout, workload, results);
        bench::printChecks(std::cout, results);
        if (!report.is_open())
            return;
        bench::printTable(report, workload, results);
        bench::printChecks(report, results);
    }
}

// Точка входа: таблицы в stdout и, по запросу, в файл Markdown / The entry point: tables to stdout and, on request, to a Markdown file
int main(int argc, char** argv)
{
    try
    {
        bench::Options options = bench::parseArguments(argc, argv);
        if (options.memoryEngine >= 0)
            return runMemoryMode(options);
        std::vector<std::unique_ptr<bench::XmlEngine>> engines = bench::createEngines();
        std::ofstream report;
        if (!options.report.empty())
            report.open(options.report);
        for (const bench::Workload& workload : makeWorkloads(options, ""))
            runWorkload(engines, workload, options, argv[0], report);
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 2;
    }
}
