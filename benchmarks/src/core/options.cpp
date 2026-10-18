#include "bench/options.hpp"
#include "bench/engine.hpp"

namespace bench
{
    // Значение ключа --memory: номер библиотеки и имя набора данных / The --memory value: the library number and the data set name
    static void readMemoryMode(int argc, char** argv, int& index, Options& options)
    {
        if (index + 2 >= argc)
            throw BenchError("--memory: нужны номер и набор данных / a number and a data set are needed");
        options.memoryEngine = std::stol(argv[++index]);
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
                options.runs = std::stoul(value);
            else if (key == "--items")
                options.items = std::stoul(value);
            else if (key == "--paragraphs")
                options.paragraphs = std::stoul(value);
            else if (key == "--report")
                options.report = value;
            else
                throw BenchError("неизвестный ключ / unknown key " + key);
        }
        return options;
    }
}
