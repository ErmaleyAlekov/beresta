#include "parser/internal/input_stack.hpp"
#include "i18n/i18n.hpp"

#include <string>

namespace beresta::detail
{
    // Внутри сущности — когда над основным документом есть хоть один источник
    // Inside an entity — when there is at least one source above the main document
    bool InputStack::inEntity() const noexcept
    {
        return _frames.size() > 1;
    }

    // Глубина раскрытия сущностей
    // The entity expansion depth
    std::size_t InputStack::depth() const noexcept
    {
        return _frames.size() - 1;
    }

    // Сущность уже раскрывается где-то ниже по стеку — значит, рекурсия
    // The entity is already being expanded somewhere lower in the stack — so it is recursion
    bool InputStack::isExpanding(std::string_view name) const noexcept
    {
        for (const InputFrame& frame : _frames)
        {
            if (frame.entityName == name)
                return true;
        }
        return false;
    }

    /*
    ** Новый источник получает «якорь» — позицию ссылки в основном документе:
    ** ошибка внутри текста сущности будет показана там, где сущность
    ** использована, с пояснением, о какой сущности речь.
    **
    ** A new source gets an "anchor" — the reference position in the main document:
    ** an error inside the entity text will be shown where the entity is
    ** used, with a note about which entity it is.
    */
    void InputStack::push(std::string_view name, std::string_view text, std::size_t openElements,
        std::size_t mainOffset)
    {
        ParsePosition anchor = main().computePosition(mainOffset);

        _frames.push_back(InputFrame{XmlScanner(text), name, openElements, mainOffset});
        _frames.back().scanner.setErrorAnchor(anchor, i18n::format(i18n::Message::IN_ENTITY, {name, ""}));
    }

    // Снятие вершины (основной документ не снимается)
    // Pops the top (the main document is not popped)
    void InputStack::pop() noexcept
    {
        if (_frames.size() > 1)
            _frames.pop_back();
    }
}
