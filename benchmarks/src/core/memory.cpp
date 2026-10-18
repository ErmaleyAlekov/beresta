#include "bench/measure.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#if defined(_WIN32)
    #include <windows.h>
    #include <psapi.h>
#elif defined(__linux__)
    #include <unistd.h>
#endif

namespace bench
{
#if defined(_WIN32)
    // Windows: рабочий набор процесса (резидентные страницы) / Windows: the process working set (resident pages)
    double residentBytes()
    {
        PROCESS_MEMORY_COUNTERS counters{};

        if (!GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
            return 0.0;
        return static_cast<double>(counters.WorkingSetSize);
    }
#elif defined(__linux__)
    // Linux: второе поле /proc/self/statm — резидентные страницы / Linux: the second field of /proc/self/statm — resident pages
    double residentBytes()
    {
        std::ifstream statm("/proc/self/statm");
        double total = 0.0;
        double resident = 0.0;

        if (!(statm >> total >> resident))
            return 0.0;
        return resident * static_cast<double>(sysconf(_SC_PAGESIZE));
    }
#else
    // Остальные ОС: замер памяти не поддержан / Other OSes: memory measurement is not supported
    double residentBytes()
    {
        return 0.0;
    }
#endif

    /*
    ** Память под дерево меряется в ОТДЕЛЬНОМ процессе: освобождённая память
    ** кучи обычно не возвращается системе, и в общем процессе вторая
    ** библиотека заняла бы память, уже отданную первой, — замер был бы
    ** нечестным. Дочерний процесс пишет число байтов во временный файл.
    **
    ** The tree memory is measured in a SEPARATE process: freed heap memory is
    ** usually not returned to the system, and in a shared process the second
    ** library would take memory already given up by the first one — the measurement
    ** would be unfair. The child process writes the byte count into a temporary file.
    */
    double measureMemoryInChild(const std::string& program, const std::string& engine, const std::string& workload)
    {
        std::filesystem::path file = std::filesystem::temp_directory_path() / ("beresta_bench_" + engine + ".txt");
        std::string command = "\"\"" + program + "\" --memory \"" + engine + "\" " + workload + " > \"" + file.string() + "\"\"";
        double bytes = 0.0;

#if !defined(_WIN32)
        command = command.substr(1, command.size() - 2);
#endif
        if (std::system(command.c_str()) != 0)
            return 0.0;
        std::ifstream input(file);
        input >> bytes;
        input.close();
        std::filesystem::remove(file);
        return bytes;
    }
}
