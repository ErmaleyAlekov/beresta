#ifndef CONSOLE_HPP
#define CONSOLE_HPP

namespace platform
{
    // Переключает консоль Windows в UTF-8; на Linux и macOS ничего не делает
    void enableUtf8Console() noexcept;
}

#endif
