#ifndef BERESTA_BENCH_MEASURE_HPP
#define BERESTA_BENCH_MEASURE_HPP

#include "bench/engine.hpp"
#include "bench/workload.hpp"

#include <cstddef>
#include <string>

namespace bench
{
    // Замеряемая операция / A measured operation
    enum class Operation
    {
        PARSE, TRAVERSE, SERIALIZE, XPATH, VALIDATE
    };

    // Число операций (для таблиц) / The number of operations (for tables)
    constexpr std::size_t OPERATION_COUNT = 5;

    /*
    ** Итог одной операции: медиана времени по нескольким повторам (медиана
    ** устойчива к случайным задержкам ОС лучше среднего) и результат
    ** операции для сверки между библиотеками. supported = false — библиотека
    ** операцию не умеет.
    **
    ** The outcome of one operation: the median time over several runs (the
    ** median is more robust to random OS delays than the mean) and the operation
    ** result for comparison between the libraries. supported = false — the library
    ** cannot do the operation.
    */
    struct Measurement
    {
        bool supported;
        double seconds;
        std::size_t result;
    };

    // Все операции одной библиотеки над одним набором данных / All operations of one library on one data set
    struct EngineResult
    {
        std::string engine;
        Measurement operations[OPERATION_COUNT];
        double memoryBytes;
    };

    // ---- Время (measure.cpp) / Time ----
    Measurement measure(XmlEngine& engine, Operation operation, const Workload& workload, std::size_t runs);

    // ---- Память (memory.cpp) / Memory ----

    // Резидентная память процесса в байтах (0 — неизвестно на этой ОС)
    // The resident memory of the process in bytes (0 — unknown on this OS)
    double residentBytes();

    // Прирост памяти от дерева DOM в отдельном процессе / The memory growth from a DOM tree in a separate process
    double measureMemoryInChild(const std::string& program, const std::string& engine, const std::string& workload);
}

#endif
