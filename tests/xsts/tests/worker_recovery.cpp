#include "xsts/case_timeout.hpp"

#include <cstdlib>
#include <string>

// Безобидные имитации аварии и долгого случая / Simulate a worker exit and a slow case
int main(int argc, char** argv)
{
    if (argc < 3 || std::string(argv[1]) != "--worker")
        return xsts::runSupervised(argc, argv);
    xsts::CaseTimeout guard(std::filesystem::u8path(argv[2]));
    if (guard.start(7))
        std::_Exit(3);
    if (guard.failure(7).find("аварийное") == std::string::npos)
        return 1;
    if (guard.start(8))
    {
        std::this_thread::sleep_for(std::chrono::seconds(65));
        return 1;
    }
    if (guard.failure(8).find("тайм-аут") == std::string::npos || !guard.start(9))
        return 1;
    guard.finish();
    return 0;
}
