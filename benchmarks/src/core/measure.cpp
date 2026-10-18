#include "bench/measure.hpp"

#include <algorithm>
#include <chrono>
#include <vector>

namespace bench
{
    /*
    ** Один запуск операции. Разбор каждый раз начинается с освобождения
    ** прошлого дерева, поэтому замер включает и построение дерева, и его
    ** разрушение предыдущего — как в реальной программе, разбирающей поток файлов.
    **
    ** One run of an operation. Parsing always starts with freeing the
    ** previous tree, so the measurement includes building the tree and destroying
    ** the previous one — as in a real program that parses a stream of files.
    */
    static std::optional<std::size_t> runOnce(XmlEngine& engine, Operation operation, const Workload& workload)
    {
        switch (operation)
        {
            case Operation::PARSE:
                engine.parse(workload.xml);
                return engine.traverse() > 0;
            case Operation::TRAVERSE:
                return engine.traverse();
            case Operation::SERIALIZE:
                return engine.serialize();
            case Operation::XPATH:
                return engine.xpathCount(workload.query);
            case Operation::VALIDATE:
                if (workload.xsd.empty())
                    return std::nullopt;
                return engine.parseValidated(workload.xml, workload.xsd);
            case Operation::STREAM_VALIDATE:
                if (workload.xsd.empty())
                    return std::nullopt;
                return engine.streamValidated(workload.xml, workload.xsd);
        }
        return std::nullopt;
    }

    // Медиана: устойчива к единичным задержкам планировщика ОС / The median: robust to single OS scheduler delays
    static double median(std::vector<double> values)
    {
        std::sort(values.begin(), values.end());
        return values[values.size() / 2];
    }

    /*
    ** Разбор измеряется отдельно от обхода: перед замером PARSE дерево
    ** строится заново, а время обхода при этом не учитывается. Первый
    ** запуск — прогрев (кэши, ленивые таблицы библиотек) — в медиану не входит.
    **
    ** Parsing is measured separately from the walk: before the PARSE measurement
    ** the tree is built anew, and the walk time is not counted. The first run is a
    ** warm-up (caches, lazy library tables) and does not go into the median.
    */
    Measurement measure(XmlEngine& engine, Operation operation, const Workload& workload, std::size_t runs)
    {
        std::vector<double> times;
        std::optional<std::size_t> result = runOnce(engine, operation, workload);

        if (!result.has_value())
            return Measurement{false, 0.0, 0};
        for (std::size_t run = 0; run < runs; ++run)
        {
            auto start = std::chrono::steady_clock::now();
            if (operation == Operation::PARSE)
                engine.parse(workload.xml);
            else
                result = runOnce(engine, operation, workload);
            std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
            times.push_back(elapsed.count());
        }
        return Measurement{true, median(times), result.value_or(0)};
    }
}
