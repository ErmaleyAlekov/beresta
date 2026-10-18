#include "parser/xml_scanner.hpp"

namespace beresta::detail
{
    // Текущее смещение курсора
    // The current cursor offset
    std::size_t XmlScanner::getOffset() const noexcept
    {
        return _offset;
    }

    // Указатели внутри string_view-срезов позволяют восстановить точную позицию ошибки
    // Pointers inside string_view slices allow restoring the exact error position
    std::size_t XmlScanner::offsetOf(const char* pointer) const noexcept
    {
        return static_cast<std::size_t>(pointer - _input.data());
    }

    // Ошибка в текущей позиции курсора
    // An error at the current cursor position
    void XmlScanner::fail(const std::string& message) const
    {
        failAt(message, _offset);
    }

    // Синтаксическая ошибка в произвольной позиции (например, в начале тега)
    // A syntax error at an arbitrary position (for example, at the start of a tag)
    void XmlScanner::failAt(const std::string& message, std::size_t offset) const
    {
        failAt(message, offset, DomErrorCode::PARSE_ERROR);
    }

    /*
    ** Если задан якорь (сканер читает текст раскрытой сущности), ошибка
    ** сообщается в позиции ссылки на сущность, а сообщение дополняется
    ** контекстом «в сущности &имя;» — так её легко найти в исходном файле.
    **
    ** If an anchor is set (the scanner reads the text of an expanded entity), the error
    ** is reported at the position of the entity reference, and the message is extended
    ** with the context "in the entity &name;" — so it is easy to find in the source file.
    */
    void XmlScanner::failAt(const std::string& message, std::size_t offset,
        DomErrorCode code) const
    {
        if (_anchor.has_value())
            throw ParseException(_context + message, _anchor.value(), code);
        throw ParseException(message, computePosition(offset), code);
    }
}
