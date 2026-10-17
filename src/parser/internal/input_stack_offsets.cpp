#include "parser/internal/input_stack.hpp"

namespace beresta::detail
{
    /*
    ** Узлы, пришедшие из сущности, получают позицию самой внешней ссылки
    ** в основном документе — именно её пользователь видит в своём файле.
    */
    std::size_t InputStack::mainOffsetFor(std::size_t offset) const noexcept
    {
        if (_frames.size() == 1)
            return offset;
        return _frames[1].mainOffset;
    }
}
