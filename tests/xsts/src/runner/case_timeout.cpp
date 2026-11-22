#include "xsts/case_timeout.hpp"
#include "xsts/xsts_error.hpp"

#include <cstdlib>
#include <fstream>

namespace xsts
{
    CaseTimeout::CaseTimeout(const std::filesystem::path& journal)
    {
        std::ifstream input(journal);
        std::size_t index;
        for (int status; input >> index >> status;)
            _failures.emplace(index, status);
        _active.exceptions(std::ios::failbit | std::ios::badbit);
        _active.open(journal.parent_path() / "active", std::ios::binary);
        finish();
        _monitor = std::thread(&CaseTimeout::monitor, this);
    }

    CaseTimeout::~CaseTimeout()
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _stopped = true;
        }
        _changed.notify_one();
        _monitor.join();
    }

    std::string CaseTimeout::failure(std::size_t index) const
    {
        return _failures.at(index) == 124 ? "тайм-аут случая (60 с)" : "аварийное завершение рабочего процесса";
    }

    bool CaseTimeout::start(std::size_t index)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_failures.count(index))
            return false;
        _active.seekp(0);
        _active.write(reinterpret_cast<const char*>(&index), sizeof(index)).flush();
        _current = index;
        _deadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
        _changed.notify_one();
        return true;
    }

    void CaseTimeout::finish()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        const std::size_t inactive = static_cast<std::size_t>(-1);
        _active.seekp(0);
        _active.write(reinterpret_cast<const char*>(&inactive), sizeof(inactive)).flush();
        _current.reset();
        _changed.notify_one();
    }

    // Завершается только рабочий процесс / Only the worker exits on timeout
    void CaseTimeout::monitor()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (!_stopped)
        {
            if (!_current)
                _changed.wait(lock);
            else
            {
                auto deadline = _deadline;
                if (_changed.wait_until(lock, deadline) == std::cv_status::timeout
                    && _current && std::chrono::steady_clock::now() >= _deadline)
                {
                    std::_Exit(124);
                }
            }
        }
    }
}
