#include "parser/xml_scanner.hpp"

namespace beresta::detail
{
    // Текущее смещение курсора
    std::size_t XmlScanner::getOffset() const noexcept
    {
        return _offset;
    }

    // Указатели внутри string_view-срезов позволяют восстановить точную позицию ошибки
    std::size_t XmlScanner::offsetOf(const char* pointer) const noexcept
    {
        return static_cast<std::size_t>(pointer - _input.data());
    }

    // Ошибка в текущей позиции курсора
    void XmlScanner::fail(const std::string& message) const
    {
        failAt(message, _offset);
    }

    // Синтаксическая ошибка в произвольной позиции (например, в начале тега)
    void XmlScanner::failAt(const std::string& message, std::size_t offset) const
    {
        failAt(message, offset, DomErrorCode::PARSE_ERROR);
    }

    /*
    ** Если задан якорь (сканер читает текст раскрытой сущности), ошибка
    ** сообщается в позиции ссылки на сущность, а сообщение дополняется
    ** контекстом «в сущности &имя;» — так её легко найти в исходном файле.
    */
    void XmlScanner::failAt(const std::string& message, std::size_t offset,
        DomErrorCode code) const
    {
        if (_anchor.has_value())
            throw ParseException(_context + message, _anchor.value(), code);
        throw ParseException(message, computePosition(offset), code);
    }
}
