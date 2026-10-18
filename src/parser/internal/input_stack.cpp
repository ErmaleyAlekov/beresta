#include "parser/internal/input_stack.hpp"

#include <string>

namespace beresta::detail
{
    // Основной документ — нижний элемент стека, он не снимается никогда
    // The main document is the bottom element of the stack, it is never popped
    InputStack::InputStack(std::string_view mainText)
        : _frames()
    {
        _frames.push_back(InputFrame{XmlScanner(mainText), std::string_view(), 0, 0});
    }

    // Источник, из которого сейчас идёт чтение
    // The source being read now
    XmlScanner& InputStack::current() noexcept
    {
        return _frames.back().scanner;
    }

    // Основной документ
    // The main document
    XmlScanner& InputStack::main() noexcept
    {
        return _frames.front().scanner;
    }

    // Основной документ (константный доступ — для вычисления позиций)
    // The main document (const access — for computing positions)
    const XmlScanner& InputStack::main() const noexcept
    {
        return _frames.front().scanner;
    }

    // Вершина стека
    // The top of the stack
    const InputFrame& InputStack::top() const noexcept
    {
        return _frames.back();
    }
}
