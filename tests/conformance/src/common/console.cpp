#include "conformance/console.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace conformance
{
    // Консоль Windows по умолчанию не в UTF-8 — без переключения кириллица отчёта нечитаема
    // The Windows console is not in UTF-8 by default — without switching the Cyrillic in the report is unreadable
    void enableUtf8Console() noexcept
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
#endif
    }
}
