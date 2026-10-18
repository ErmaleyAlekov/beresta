#ifndef LOCATOR_HPP
#define LOCATOR_HPP

#include "core/source_location.hpp"

namespace beresta
{
    /*
    ** Locator — источник текущей позиции разбора (аналог Locator в SAX).
    ** Парсер передаёт его обработчику в setDocumentLocator(); во время
    ** любого события getLocation() возвращает начало текущей конструкции.
    ** Позиция вычисляется лениво, только когда её действительно запросили.
    **
    ** Locator — the source of the current parse position (the analog of Locator in SAX).
    ** The parser passes it to the handler in setDocumentLocator(); during
    ** any event getLocation() returns the start of the current construct.
    ** The position is computed lazily, only when it is actually requested.
    */
    class Locator
    {
    public:
        virtual ~Locator();

        virtual SourceLocation getLocation() const = 0;
    };
}

#endif
