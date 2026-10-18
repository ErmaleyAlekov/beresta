#include "content/content_cursor.hpp"
#include "content/internal/automaton_step.hpp"

#include <utility>

namespace beresta::content
{
    /*
    ** Шаг автомата: все продвижения всех текущих конфигураций на одного
    ** ребёнка. Пустой результат — ребёнок не продолжает ни один путь по
    ** модели. Наборы меняются местами, а не копируются.
    **
    ** An automaton step: all advances of all current configurations by one
    ** child. An empty result means the child does not continue any path through the
    ** model. The sets are swapped, not copied.
    */
    bool ContentCursor::stepAutomaton(const ChildKey& child)
    {
        detail::AutomatonStep automaton(child, _next, _work);

        _next.clear();
        if (_consumed == 0)
            automaton.fromStart(*_model);
        for (std::size_t index = 0; _consumed > 0 && index < _current.count; ++index)
            automaton.fromConfig(_current.items[index]);
        if (_next.count == 0)
            return false;
        std::swap(_current, _next);
        return true;
    }

    // Модель заканчивается, если хотя бы одна конфигурация может завершиться без новых детей
    // The model ends if at least one configuration can finish without new children
    bool ContentCursor::finishAutomaton() const
    {
        if (_consumed == 0)
            return canFinishConfig(*_model, MatchConfig());
        for (std::size_t index = 0; index < _current.count; ++index)
        {
            if (canFinishConfig(*_model, _current.items[index]))
                return true;
        }
        return false;
    }

    // Отложенный режим: ребёнок копируется (представления SAX живут только до конца события)
    // The deferred mode: the child is copied (SAX views live only until the end of the event)
    void ContentCursor::stepBuffered(const ChildKey& child)
    {
        if (_bufferedCount == _buffered.size())
            _buffered.emplace_back();
        OwnedChildKey& slot = _buffered[_bufferedCount++];
        slot.qualifiedName.assign(child.qualifiedName);
        slot.localName.assign(child.localName);
        slot.namespaceUri.assign(child.namespaceUri);
    }

    // Отложенный режим: весь список детей — общему алгоритму на множествах позиций
    // The deferred mode: the whole child list goes to the general algorithm on position sets
    MatchResult ContentCursor::finishBuffered()
    {
        std::vector<ChildKey> keys;

        keys.reserve(_bufferedCount);
        for (std::size_t index = 0; index < _bufferedCount; ++index)
            keys.push_back(ChildKey{_buffered[index].qualifiedName, _buffered[index].localName,
                _buffered[index].namespaceUri});
        return matchContent(*_model, keys);
    }
}
