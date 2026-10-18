#include "parser/xml_scanner.hpp"

namespace beresta::detail
{
    // Сканер начинает с самого начала ввода
    // The scanner starts at the very beginning of the input
    XmlScanner::XmlScanner(std::string_view input) noexcept
        : _input(input), _offset(0), _cache{0, 1, 1}, _anchor(), _context()
    {
    }

    // Конец ввода — когда смещение дошло до длины строки
    // The end of input — when the offset reaches the string length
    bool XmlScanner::isAtEnd() const noexcept
    {
        return _offset >= _input.size();
    }

    // '\0' на конце избавляет вызывающий код от отдельной проверки isAtEnd()
    // '\0' at the end saves the calling code a separate isAtEnd() check
    char XmlScanner::peek() const noexcept
    {
        if (isAtEnd())
            return '\0';
        return _input[_offset];
    }

    // Сравнение среза остатка с фрагментом (без копирования)
    // Compares a slice of the rest with a fragment (without copying)
    bool XmlScanner::startsWith(std::string_view fragment) const noexcept
    {
        if (_input.size() - _offset < fragment.size())
            return false;
        return _input.substr(_offset, fragment.size()) == fragment;
    }

    // Сдвиг с ограничением, чтобы курсор никогда не вышел за пределы ввода
    // A move with a limit so that the cursor never leaves the input
    void XmlScanner::advance(std::size_t count) noexcept
    {
        std::size_t remaining = _input.size() - _offset;

        if (count > remaining)
            count = remaining;
        _offset += count;
    }
}
