#include "parser/xml_scanner.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::detail
{
    // Обязательный фрагмент синтаксиса: либо он есть, либо понятная ошибка
    void XmlScanner::expect(std::string_view fragment)
    {
        if (!startsWith(fragment))
            fail("ожидалось \"" + std::string(fragment) + "\"");
        advance(fragment.size());
    }

    // Пробелы, табы и переводы строк пропускаются одинаково
    std::size_t XmlScanner::skipWhitespace() noexcept
    {
        std::size_t start = _offset;

        while (!isAtEnd() && utils::isWhitespace(_input[_offset]))
            ++_offset;
        return _offset - start;
    }

    // Имя: первый символ — NameStartChar, остальные — NameChar
    std::string_view XmlScanner::readName()
    {
        std::size_t start = _offset;

        if (isAtEnd() || !utils::isNameStartChar(_input[_offset]))
            fail("ожидалось XML-имя");
        ++_offset;
        while (!isAtEnd() && utils::isNameChar(_input[_offset]))
            ++_offset;
        return _input.substr(start, _offset - start);
    }

    // Срез до разделителя; сам разделитель пропускается
    std::string_view XmlScanner::readUntil(std::string_view delimiter)
    {
        std::size_t found = _input.find(delimiter, _offset);

        if (found == std::string_view::npos)
            fail("не найдено закрывающее \"" + std::string(delimiter) + "\"");
        std::string_view content = _input.substr(_offset, found - _offset);
        _offset = found + delimiter.size();
        return content;
    }

    // Срез до символа или до конца ввода; сам символ не пропускается
    std::string_view XmlScanner::readUntilChar(char symbol) noexcept
    {
        std::size_t found = _input.find(symbol, _offset);

        if (found == std::string_view::npos)
            found = _input.size();
        std::string_view content = _input.substr(_offset, found - _offset);
        _offset = found;
        return content;
    }
}
