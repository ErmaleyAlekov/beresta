#include "parser/xml_scanner.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::detail
{
    // Обязательный фрагмент синтаксиса: либо он есть, либо понятная ошибка
    // A mandatory syntax fragment: either it is there or there is a clear error
    void XmlScanner::expect(std::string_view fragment)
    {
        if (!startsWith(fragment))
            fail(i18n::format(i18n::Message::EXPECTED_TEXT, {fragment}));
        advance(fragment.size());
    }

    // Пробелы, табы и переводы строк пропускаются одинаково
    // Spaces, tabs and line breaks are skipped the same way
    std::size_t XmlScanner::skipWhitespace() noexcept
    {
        std::size_t start = _offset;

        while (!isAtEnd() && utils::isWhitespace(_input[_offset]))
            ++_offset;
        return _offset - start;
    }

    // Имя: первый символ — NameStartChar, остальные — NameChar (таблицы 5-й редакции)
    // A name: the first character is a NameStartChar, the rest are NameChars (the 5th edition tables)
    std::string_view XmlScanner::readName()
    {
        std::size_t start = _offset;
        std::size_t length = utils::nameLength(_input, _offset);

        if (length == 0)
            fail(i18n::format(i18n::Message::EXPECTED_XML_NAME));
        _offset += length;
        return _input.substr(start, length);
    }

    // Срез до разделителя; сам разделитель пропускается
    // A slice up to a delimiter; the delimiter itself is skipped
    std::string_view XmlScanner::readUntil(std::string_view delimiter)
    {
        std::size_t found = _input.find(delimiter, _offset);

        if (found == std::string_view::npos)
            fail(i18n::format(i18n::Message::CLOSING_DELIMITER_MISSING, {delimiter}));
        std::string_view content = _input.substr(_offset, found - _offset);
        _offset = found + delimiter.size();
        return content;
    }

    // Срез до символа или до конца ввода; сам символ не пропускается
    // A slice up to a character or to the end of input; the character itself is not skipped
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
