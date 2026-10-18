#include "platform/console.hpp"

#ifdef _WIN32
# include <windows.h>
#endif

namespace platform
{
    /*
    ** Консоль Windows по умолчанию использует кодовую страницу OEM (866),
    ** и русский текст в UTF-8 выводился бы «кракозябрами».
    ** На Unix-системах терминалы работают в UTF-8 изначально.
    **
    ** The Windows console uses the OEM code page (866) by default,
    ** and Russian text in UTF-8 would be printed as garbage.
    ** On Unix systems terminals work in UTF-8 from the start.
    */
    void enableUtf8Console() noexcept
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
#endif
    }
}
