#include "bench/options.hpp"
#include "bench/engine.hpp"

#include <charconv>
#include <string_view>

namespace bench
{
    // Целое число без знака и лишних символов / An unsigned decimal without trailing characters
    template<class T>
    static T number(std::string_view value)
    {
        T result{};
        auto end = value.data() + value.size();
        auto parsed = std::from_chars(value.data(), end, result);
        if (value.empty() || value.front() == '-' || parsed.ec != std::errc() || parsed.ptr != end)
            throw BenchError("неверное число / invalid number: " + std::string(value));
        return result;
    }

    // Значение ключа --memory: номер библиотеки и имя набора данных / The --memory value: the library number and the data set name
    static void readMemoryMode(int argc, char** argv, int& index, Options& options)
    {
        if (index + 2 >= argc)
            throw BenchError("--memory: нужны номер и набор данных / a number and a data set are needed");
        options.memoryEngine = number<decltype(options.memoryEngine)>(argv[++index]);
        options.memoryWorkload = argv[++index];
    }

    /*
    ** Ключи: --runs N, --items N, --paragraphs N, --report файл.md и служебный
    ** --memory <номер> <набор>. Каждый ключ требует значения.
    **
    ** Keys: --runs N, --items N, --paragraphs N, --report file.md and the service
    ** --memory <number> <data set>. Every key requires a value.
    */
    Options parseArguments(int argc, char** argv)
    {
        Options options;

        for (int index = 1; index < argc; ++index)
        {
            std::string key = argv[index];
            if (key == "--memory")
            {
                readMemoryMode(argc, argv, index, options);
                continue;
            }
            if (index + 1 >= argc)
                throw BenchError("нет значения для / no value for " + key);
            std::string value = argv[++index];
            if (key == "--runs")
                options.runs = number<decltype(options.runs)>(value);
            else if (key == "--items")
                options.items = number<decltype(options.items)>(value);
            else if (key == "--paragraphs")
                options.paragraphs = number<decltype(options.paragraphs)>(value);
            else if (key == "--memory-output")
                options.memoryOutput = value;
            else if (key == "--report")
                options.report = value;
            else
                throw BenchError("неизвестный ключ / unknown key " + key);
        }
        if (options.runs == 0)
            throw BenchError("--runs: требуется положительное число / expected positive count");
        return options;
    }
}
