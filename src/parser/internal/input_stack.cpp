#include "parser/internal/input_stack.hpp"

#include <string>

namespace beresta::detail
{
    // Основной документ — нижний элемент стека, он не снимается никогда
    InputStack::InputStack(std::string_view mainText)
        : _frames()
    {
        _frames.push_back(InputFrame{XmlScanner(mainText), std::string_view(), 0, 0});
    }

    // Источник, из которого сейчас идёт чтение
    XmlScanner& InputStack::current() noexcept
    {
        return _frames.back().scanner;
    }

    // Основной документ
    XmlScanner& InputStack::main() noexcept
    {
        return _frames.front().scanner;
    }

    // Основной документ (константный доступ — для вычисления позиций)
    const XmlScanner& InputStack::main() const noexcept
    {
        return _frames.front().scanner;
    }

    // Вершина стека
    const InputFrame& InputStack::top() const noexcept
    {
        return _frames.back();
    }
}
