#include "parser/xml_scanner.hpp"

namespace beresta::detail
{
    // Сканер начинает с самого начала ввода
    XmlScanner::XmlScanner(std::string_view input) noexcept
        : _input(input), _offset(0), _cache{0, 1, 1}, _anchor(), _context()
    {
    }

    // Конец ввода — когда смещение дошло до длины строки
    bool XmlScanner::isAtEnd() const noexcept
    {
        return _offset >= _input.size();
    }

    // '\0' на конце избавляет вызывающий код от отдельной проверки isAtEnd()
    char XmlScanner::peek() const noexcept
    {
        if (isAtEnd())
            return '\0';
        return _input[_offset];
    }

    // Сравнение среза остатка с фрагментом (без копирования)
    bool XmlScanner::startsWith(std::string_view fragment) const noexcept
    {
        if (_input.size() - _offset < fragment.size())
            return false;
        return _input.substr(_offset, fragment.size()) == fragment;
    }

    // Сдвиг с ограничением, чтобы курсор никогда не вышел за пределы ввода
    void XmlScanner::advance(std::size_t count) noexcept
    {
        std::size_t remaining = _input.size() - _offset;

        if (count > remaining)
            count = remaining;
        _offset += count;
    }
}
