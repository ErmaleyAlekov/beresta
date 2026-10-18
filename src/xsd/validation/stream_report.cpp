#include "xsd/internal/stream_validator.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** Ошибка у открытого элемента уровня level: владелец в ключе порядка —
    ** сам элемент, шаг пути закрепляется только сейчас (у элементов без
    ** ошибок путей нет вовсе).
    **
    ** An error at the open element of the level level: the owner in the order key is
    ** the element itself, the path step is pinned only now (elements without
    ** errors have no paths at all).
    */
    void StreamValidator::report(std::size_t level, CheckPhase phase, std::string message)
    {
        const ElementFrame& frame = _frames[level];

        _log.add(DiagnosticOrder{static_cast<std::size_t>(OrderGroup::ELEMENTS), frame.ordinal,
            static_cast<std::size_t>(phase), 0, 0}, _paths.pin(level), std::move(message), frame.position);
    }

    /*
    ** Ошибка, найденная родителем у ребёнка (лишний по модели ребёнок,
    ** необъявленный при xs:any strict): путь и позиция — ребёнка, а место
    ** в порядке — среди проверок родителя, до собственных ошибок ребёнка.
    **
    ** An error found by the parent at a child (a child extra by the model,
    ** one undeclared with xs:any strict): the path and the position are the child's, and the place
    ** in the order is among the parent's checks, before the child's own errors.
    */
    void StreamValidator::reportAtChild(CheckPhase phase, std::string message)
    {
        const ElementFrame& parent = _frames[_depth - 2];
        const ElementFrame& child = _frames[_depth - 1];

        _log.add(DiagnosticOrder{static_cast<std::size_t>(OrderGroup::ELEMENTS), parent.ordinal,
            static_cast<std::size_t>(phase), 0, 0}, _paths.pin(_depth - 1), std::move(message), child.position);
    }
}
