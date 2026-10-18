#include "xsd/internal/diagnostic_log.hpp"

#include <algorithm>

namespace beresta::xsd::detail
{
    // Порядок меток: от начала входа к концу
    // The order of marks: from the start of the input to its end
    static bool markBefore(const EventPosition* left, const EventPosition* right) noexcept
    {
        return left->mark < right->mark;
    }

    /*
    ** Позиции считаются по возрастанию меток: парсер досчитывает строки от
    ** прошлой вычисленной позиции, поэтому весь перевод — один проход по
    ** входу, сколько бы ошибок ни было. Переведённая метка снимается, чтобы
    ** повторный вызов ничего не пересчитывал.
    **
    ** The positions are computed in ascending order of marks: the parser counts lines on from
    ** the last computed position, so the whole conversion is one pass over the
    ** input, however many errors there are. A converted mark is removed so that
    ** a repeated call recomputes nothing.
    */
    void resolvePositions(std::vector<EventPosition*>& positions, const Locator& locator)
    {
        std::vector<EventPosition*> marked;

        for (EventPosition* position : positions)
        {
            if (position->mark != Locator::NO_MARK)
                marked.push_back(position);
        }
        std::sort(marked.begin(), marked.end(), markBefore);
        for (EventPosition* position : marked)
        {
            position->location = locator.locate(position->mark);
            position->mark = Locator::NO_MARK;
        }
    }

    // Метки всех накопленных сообщений
    // The marks of all accumulated messages
    void DiagnosticLog::resolve(const Locator& locator)
    {
        std::vector<EventPosition*> positions;

        positions.reserve(_pending.size());
        for (PendingDiagnostic& pending : _pending)
            positions.push_back(&pending.position);
        resolvePositions(positions, locator);
    }
}
