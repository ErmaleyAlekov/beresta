#include "sax/locator.hpp"

namespace beresta
{
    // Виртуальный деструктор интерфейса Locator
    // The virtual destructor of the Locator interface
    Locator::~Locator()
    {
    }

    // Источник без меток: позицию можно получить только сразу, через getLocation()
    // A source without marks: the position can be obtained only at once, through getLocation()
    std::size_t Locator::getMark() const
    {
        return NO_MARK;
    }

    // Без меток и позиции по ним нет
    // Without marks there is no position by them either
    SourceLocation Locator::locate(std::size_t mark) const
    {
        (void)mark;
        return SourceLocation();
    }
}
