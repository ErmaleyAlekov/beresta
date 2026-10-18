#ifndef BERESTA_BENCH_REPORT_HPP
#define BERESTA_BENCH_REPORT_HPP

#include "bench/measure.hpp"

#include <ostream>
#include <vector>

namespace bench
{
    // Таблица Markdown по одному набору данных / A Markdown table for one data set
    void printTable(std::ostream& output, const Workload& workload, const std::vector<EngineResult>& results);

    // Сверка контрольных сумм: одинаковое ли дерево построили библиотеки
    // Checksum comparison: whether the libraries built the same tree
    void printChecks(std::ostream& output, const std::vector<EngineResult>& results);
}

#endif
