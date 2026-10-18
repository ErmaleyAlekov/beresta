#ifndef CONSOLE_HPP
#define CONSOLE_HPP

namespace platform
{
    // Переключает консоль Windows в UTF-8; на Linux и macOS ничего не делает
    // Switches the Windows console to UTF-8; does nothing on Linux and macOS
    void enableUtf8Console() noexcept;
}

#endif
