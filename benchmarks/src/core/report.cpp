#include "bench/report.hpp"

#include <cstdio>
#include <string>

namespace bench
{
    // Пропускная способность в МБ/с (миллионы байтов в секунду) или «—»
    // Throughput in MB/s (millions of bytes per second) or "—"
    static std::string throughput(const Measurement& measurement, double bytes)
    {
        char buffer[32];

        if (!measurement.supported || measurement.seconds <= 0.0)
            return "—";
        std::snprintf(buffer, sizeof(buffer), "%.0f", bytes / measurement.seconds / 1e6);
        return buffer;
    }

    // Время в миллисекундах или «—» / Time in milliseconds or "—"
    static std::string milliseconds(const Measurement& measurement)
    {
        char buffer[32];

        if (!measurement.supported)
            return "—";
        std::snprintf(buffer, sizeof(buffer), "%.1f", measurement.seconds * 1000.0);
        return buffer;
    }

    // Память дерева: мегабайты и кратность размеру входа / The tree memory: megabytes and the ratio to the input size
    static std::string memory(double bytes, double input)
    {
        char buffer[48];

        if (bytes <= 0.0)
            return "—";
        std::snprintf(buffer, sizeof(buffer), "%.0f (×%.1f)", bytes / 1e6, bytes / input);
        return buffer;
    }

    /*
    ** Таблица по набору данных. Разбор и проверка по схеме (по дереву и
    ** потоком SAX) — в МБ/с от
    ** размера входа (больше — лучше), обход и XPath — в миллисекундах на всё
    ** дерево (меньше — лучше), сериализация — в МБ/с от размера вывода.
    **
    ** A table for a data set. Parsing and schema validation (over a tree and
    ** by a SAX stream) are in MB/s of the
    ** input size (more is better), the walk and XPath are in milliseconds for the
    ** whole tree (less is better), serialization is in MB/s of the output size.
    */
    void printTable(std::ostream& output, const Workload& workload, const std::vector<EngineResult>& results)
    {
        double input = static_cast<double>(workload.xml.size());

        output << "\n### " << workload.name << " (" << static_cast<long>(input / 1e6) << " MB)\n\n"
               << "| Библиотека / Library | Разбор / Parse, MB/s | Обход / Walk, ms | Вывод / Serialize, MB/s"
               << " | XPath, ms | Разбор + XSD / Parse + XSD, MB/s | SAX + XSD, MB/s | Память DOM / DOM memory, MB |\n"
               << "|---|---|---|---|---|---|---|---|\n";
        for (const EngineResult& result : results)
        {
            const Measurement* m = result.operations;
            output << "| " << result.engine << " | " << throughput(m[0], input) << " | " << milliseconds(m[1])
                   << " | " << throughput(m[2], static_cast<double>(m[2].result)) << " | " << milliseconds(m[3])
                   << " | " << throughput(m[4], input) << " | " << throughput(m[5], input)
                   << " | " << memory(result.memoryBytes, input) << " |\n";
        }
    }
}
