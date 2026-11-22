#include "xsts/case_timeout.hpp"
#include "xsts/xsts_error.hpp"

#include <cerrno>
#include <random>
#include <vector>

#ifdef _WIN32
    #include <process.h>
#else
    #include <spawn.h>
    #include <sys/wait.h>
    extern char** environ;
#endif

namespace xsts
{
#ifdef _WIN32
    // Экранирование аргумента по правилам CRT / Quote one argument for the Windows CRT
    static std::wstring quote(const std::wstring& arg)
    {
        std::wstring result = L"\"";
        std::size_t slashes = 0;
        for (wchar_t ch : arg)
        {
            if (ch == L'\\')
                ++slashes;
            else
            {
                result.append(ch == L'"' ? 2 * slashes + 1 : slashes, L'\\');
                result += ch;
                slashes = 0;
            }
        }
        return result.append(2 * slashes, L'\\') + L'"';
    }

    static int launch(const std::vector<std::string>& args)
    {
        std::vector<std::wstring> quoted;
        for (const auto& arg : args)
            quoted.push_back(quote(std::filesystem::u8path(arg).wstring()));
        std::vector<const wchar_t*> argv;
        for (const auto& arg : quoted)
            argv.push_back(arg.c_str());
        argv.push_back(nullptr);
        return static_cast<int>(_wspawnvp(_P_WAIT, std::filesystem::u8path(args.front()).c_str(), argv.data()));
    }
#else
    static int launch(const std::vector<std::string>& args)
    {
        std::vector<char*> argv;
        for (const auto& arg : args)
            argv.push_back(const_cast<char*>(arg.c_str()));
        argv.push_back(nullptr);
        pid_t pid;
        if (posix_spawnp(&pid, argv.front(), nullptr, nullptr, argv.data(), environ) != 0)
            return -1;
        int status;
        while (waitpid(pid, &status, 0) < 0)
            if (errno != EINTR) return -1;
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
#endif

    // В журнал попадают только аварии внутри конкретного случая / Record crashes only inside a case
    static bool recordFailure(const std::filesystem::path& journal, int status)
    {
        std::size_t index;
        auto marker = journal.parent_path() / "active";
        std::ifstream active(marker, std::ios::binary);
        if (!active.read(reinterpret_cast<char*>(&index), sizeof(index)) || index == static_cast<std::size_t>(-1))
            return false;
        active.close();
        std::filesystem::remove(marker);
        std::ofstream output(journal, std::ios::app);
        output << index << ' ' << status << '\n';
        output.close();
        return static_cast<bool>(output);
    }

    int runSupervised(int argc, char** argv)
    {
        auto folder = std::filesystem::temp_directory_path() / ("beresta_xsts_" + std::to_string(std::random_device{}()));
        if (!std::filesystem::create_directory(folder))
            throw XstsError("не удалось создать каталог журнала");
        auto file = folder / "timeouts";
        std::vector<std::string> args{argv[0], "--worker", file.u8string()};
        args.insert(args.end(), argv + 1, argv + argc);
        int status;
        do
            status = launch(args);
        while ((status < 0 || status > 2) && recordFailure(file, status));
        std::filesystem::remove(folder / "active");
        std::filesystem::remove(file);
        std::filesystem::remove(folder);
        if (status < 0)
            throw XstsError("сбой рабочего процесса");
        return status;
    }
}
