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
    */
    void enableUtf8Console() noexcept
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
#endif
    }
}
