#include "parser/internal/input_stack.hpp"

#include <string>

namespace beresta::detail
{
    // Внутри сущности — когда над основным документом есть хоть один источник
    bool InputStack::inEntity() const noexcept
    {
        return _frames.size() > 1;
    }

    // Глубина раскрытия сущностей
    std::size_t InputStack::depth() const noexcept
    {
        return _frames.size() - 1;
    }

    // Сущность уже раскрывается где-то ниже по стеку — значит, рекурсия
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
    */
    void InputStack::push(std::string_view name, std::string_view text, std::size_t openElements,
        std::size_t mainOffset)
    {
        ParsePosition anchor = main().computePosition(mainOffset);

        _frames.push_back(InputFrame{XmlScanner(text), name, openElements, mainOffset});
        _frames.back().scanner.setErrorAnchor(anchor, "в сущности &" + std::string(name) + ";: ");
    }

    // Снятие вершины (основной документ не снимается)
    void InputStack::pop() noexcept
    {
        if (_frames.size() > 1)
            _frames.pop_back();
    }
}
