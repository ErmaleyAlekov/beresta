#include "bench/report.hpp"

#include <string>

namespace bench
{
    // Результат операции числом или «—», если операция не поддержана
    // The operation result as a number or "—" if the operation is not supported
    static std::string resultCell(const Measurement& measurement)
    {
        if (!measurement.supported)
            return "—";
        return std::to_string(measurement.result);
    }

    // Обход, XPath и валидация должны совпасть / Walk, XPath and validation must agree
    static void verifyChecks(const std::vector<EngineResult>& results)
    {
        for (Operation op : {Operation::TRAVERSE, Operation::XPATH, Operation::VALIDATE, Operation::STREAM_VALIDATE})
        {
            std::optional<std::size_t> expected;
            for (const auto& engine : results)
            {
                const auto& measured = engine.operations[static_cast<std::size_t>(op)];
                if (!measured.supported)
                    continue;
                if (expected && measured.result != *expected)
                    throw BenchError("расхождение результатов / results differ: " + engine.engine);
                expected = measured.result;
            }
        }
    }

    /*
    ** Сверка результатов: контрольная сумма обхода, размер вывода, число
    ** узлов XPath и число ошибок схемы. Совпадение означает, что библиотеки
    ** построили одно и то же дерево и посчитали одно и то же.
    **
    ** Result comparison: the walk checksum, the output size, the number of XPath
    ** nodes and the number of schema errors. A match means that the libraries
    ** built the same tree and computed the same thing.
    */
    void printChecks(std::ostream& output, const std::vector<EngineResult>& results)
    {
        output << "\n| Библиотека / Library | Обход / Walk | Вывод / Output, bytes | XPath | Ошибки XSD / XSD errors | Поток / Stream |\n"
               << "|---|---|---|---|---|---|\n";
        for (const EngineResult& result : results)
        {
            const Measurement* m = result.operations;
            output << "| " << result.engine << " | " << resultCell(m[1]) << " | " << resultCell(m[2])
                   << " | " << resultCell(m[3]) << " | " << resultCell(m[4]) << " | " << resultCell(m[5]) << " |\n";
        }
        verifyChecks(results);
    }
}
