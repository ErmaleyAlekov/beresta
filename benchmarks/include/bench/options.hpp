#ifndef BERESTA_BENCH_OPTIONS_HPP
#define BERESTA_BENCH_OPTIONS_HPP

#include <cstddef>
#include <string>

namespace bench
{
    // Параметры запуска программы замеров / The run parameters of the benchmark
    struct Options
    {
        std::size_t runs = 5;
        std::size_t items = 100000;
        std::size_t paragraphs = 25000;
        std::string report;
        long memoryEngine = -1;
        std::string memoryWorkload;
    };

    // Разбор командной строки (options.cpp); неизвестный ключ — BenchError
    // Parsing the command line (options.cpp); an unknown key is BenchError
    Options parseArguments(int argc, char** argv);
}

#endif
