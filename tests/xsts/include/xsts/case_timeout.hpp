#ifndef XSTS_CASE_TIMEOUT_HPP
#define XSTS_CASE_TIMEOUT_HPP

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <thread>

namespace xsts
{
    // Ограничение времени одного случая в дочернем процессе / Per-case deadline in the worker
    class CaseTimeout
    {
    public:
        explicit CaseTimeout(const std::filesystem::path& journal);
        ~CaseTimeout();
        std::string failure(std::size_t index) const;
        bool start(std::size_t index);
        void finish();

    private:
        void monitor();
        std::map<std::size_t, int> _failures;
        std::ofstream _active;
        std::mutex _mutex;
        std::condition_variable _changed;
        std::optional<std::size_t> _current;
        std::chrono::steady_clock::time_point _deadline;
        bool _stopped = false;
        std::thread _monitor;
    };

    // Перезапускает прогон после тайм-аута / Restarts the runner after a timed-out case
    int runSupervised(int argc, char** argv);
}

#endif
