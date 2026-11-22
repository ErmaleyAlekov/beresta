#include "bench/measure.hpp"

#include <cerrno>
#include <random>
#include <sstream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <string>

#if defined(_WIN32)
    #include <process.h>
    #include <windows.h>
    #include <psapi.h>
#else
    #include <spawn.h>
    #include <sys/resource.h>
    #include <sys/wait.h>
    #include <unistd.h>
    extern char** environ;
    #ifdef __APPLE__
        #include <mach/mach.h>
    #endif
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
#elif defined(__APPLE__)
    double residentBytes()
    {
        mach_task_basic_info_data_t info{};
        mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &count) != KERN_SUCCESS)
            return 0.0;
        return static_cast<double>(info.resident_size);
    }
#else
    // Остальные ОС: замер памяти не поддержан / Other OSes: memory measurement is not supported
    double residentBytes()
    {
        return 0.0;
    }
#endif

    // Максимальный рабочий набор процесса / Process peak resident set
    double peakResidentBytes()
    {
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS counters{};
        if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
            return static_cast<double>(counters.PeakWorkingSetSize);
#elif defined(__linux__)
        std::ifstream status("/proc/self/status");
        for (std::string line; std::getline(status, line);)
        {
            if (line.compare(0, 6, "VmHWM:") != 0)
                continue;
            std::istringstream fields(line.substr(6));
            double kibibytes = 0;
            if (fields >> kibibytes)
                return kibibytes * 1024.0;
        }
#else
        struct rusage usage{};
        if (getrusage(RUSAGE_SELF, &usage) == 0)
        {
#ifdef __APPLE__
            return static_cast<double>(usage.ru_maxrss);
#else
            return static_cast<double>(usage.ru_maxrss) * 1024.0;
#endif
        }
#endif
        return 0.0;
    }

    // Запуск без командной оболочки / Launch without a command shell
    static bool runChild(const std::vector<std::string>& args)
    {
#ifdef _WIN32
        std::vector<std::wstring> quoted;
        for (const auto& arg : args)
            quoted.push_back(L"\"" + std::filesystem::u8path(arg).wstring() + L"\"");
        std::vector<const wchar_t*> argv;
        for (const auto& arg : quoted)
            argv.push_back(arg.c_str());
        argv.push_back(nullptr);
        return _wspawnvp(_P_WAIT, std::filesystem::u8path(args.front()).c_str(), argv.data()) == 0;
#else
        std::vector<char*> argv;
        for (const auto& arg : args)
            argv.push_back(const_cast<char*>(arg.c_str()));
        argv.push_back(nullptr);
        pid_t pid;
        if (posix_spawnp(&pid, argv.front(), nullptr, nullptr, argv.data(), environ) != 0)
            return false;
        int status;
        while (waitpid(pid, &status, 0) < 0)
            if (errno != EINTR) return false;
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
#endif
    }

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
    MemoryUsage measureMemoryInChild(const std::string& program, const std::string& engine, const std::string& workload)
    {
        auto folder = std::filesystem::temp_directory_path() / ("beresta_bench_" + std::to_string(std::random_device{}()));
        if (!std::filesystem::create_directory(folder))
            throw BenchError("занят временный каталог / temporary directory in use");
        auto file = folder / "rss.txt";
        std::vector<std::string> args{program, "--memory", engine};
        std::istringstream words(workload);
        for (std::string word; words >> word;)
            args.push_back(word);
        args.insert(args.end(), {"--memory-output", file.u8string()});
        MemoryUsage bytes;
        bool ran = runChild(args);
        std::ifstream input(file);
        bool measured = ran && static_cast<bool>(input >> bytes.residentGrowth >> bytes.peakGrowth);
        input.close();
        std::filesystem::remove(file);
        std::filesystem::remove(folder);
        if (!measured)
            throw BenchError("сбой замера памяти / memory probe failed");
        return bytes;
    }
}
