#ifndef CONFORMANCE_CONSOLE_HPP
#define CONFORMANCE_CONSOLE_HPP

namespace conformance
{
    // Переключает консоль Windows в UTF-8 (на других ОС ничего не делает)
    // Switches the Windows console to UTF-8 (does nothing on other OSes)
    void enableUtf8Console() noexcept;
}

#endif
