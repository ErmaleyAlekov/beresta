#ifndef LOCATOR_HPP
#define LOCATOR_HPP

#include "core/source_location.hpp"

#include <cstddef>

namespace beresta
{
    /*
    ** Locator — источник текущей позиции разбора (аналог Locator в SAX).
    ** Парсер передаёт его обработчику в setDocumentLocator(); во время
    ** любого события getLocation() возвращает начало текущей конструкции.
    ** Позиция вычисляется лениво, только когда её действительно запросили.
    **
    ** Для тех, кому позиция нужна лишь иногда (валидатор сообщает её только
    ** у ошибок), есть дешёвая метка: getMark() во время события и locate()
    ** позже — до конца разбора. Метки стоит переводить в позиции по
    ** возрастанию: тогда счёт строк идёт одним проходом по входу.
    **
    ** Locator — the source of the current parse position (the analog of Locator in SAX).
    ** The parser passes it to the handler in setDocumentLocator(); during
    ** any event getLocation() returns the start of the current construct.
    ** The position is computed lazily, only when it is actually requested.
    ** For those who need the position only occasionally (the validator reports it only
    ** for errors), there is a cheap mark: getMark() during an event and locate()
    ** later — until the end of parsing. Marks are best converted into positions in
    ** ascending order: then line counting goes in one pass over the input.
    */
    class Locator
    {
    public:
        // «Меток нет»: источник умеет только getLocation()
        // "No marks": the source can only do getLocation()
        static constexpr std::size_t NO_MARK = static_cast<std::size_t>(-1);

        virtual ~Locator();

        virtual SourceLocation getLocation() const = 0;

        // Метка начала текущей конструкции (по умолчанию NO_MARK)
        // The mark of the start of the current construct (NO_MARK by default)
        virtual std::size_t getMark() const;

        // Позиция по метке, полученной в этом же разборе (по умолчанию — нулевая)
        // The position by a mark obtained in the same parse (zero by default)
        virtual SourceLocation locate(std::size_t mark) const;
    };
}

#endif
