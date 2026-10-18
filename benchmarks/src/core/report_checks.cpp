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
        output << "\n| Библиотека / Library | Обход / Walk | Вывод / Output, bytes | XPath | Ошибки XSD / XSD errors |\n"
               << "|---|---|---|---|---|\n";
        for (const EngineResult& result : results)
        {
            const Measurement* m = result.operations;
            output << "| " << result.engine << " | " << resultCell(m[1]) << " | " << resultCell(m[2])
                   << " | " << resultCell(m[3]) << " | " << resultCell(m[4]) << " |\n";
        }
    }
}
