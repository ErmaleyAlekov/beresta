#include "parser/xml_scanner.hpp"

#include "utils/string_utils.hpp"

#include <cstring>
#include <utility>

namespace beresta::detail
{
    // Число символов в куске UTF-8: байты-продолжения (10xxxxxx) не считаются
    // The number of characters in a UTF-8 piece: continuation bytes (10xxxxxx) are not counted
    static std::size_t countCharacters(const char* bytes, std::size_t size) noexcept
    {
        std::size_t count = 0;

        for (std::size_t index = 0; index < size; ++index)
            count += static_cast<std::size_t>((static_cast<unsigned char>(bytes[index]) & 0xC0) != 0x80);
        return count;
    }

    /*
    ** Строка и столбец досчитываются от последней вычисленной позиции.
    ** Если запрошено смещение левее кэша — счёт начинается заново.
    ** Переводы строк ищутся memchr (векторно), а символы считаются только
    ** в хвосте после последнего перевода строки: валидатор и DomBuilder
    ** спрашивают позицию у каждого элемента, и побайтовый цикл с ветвлением
    ** по всему входу был заметной частью времени разбора. Байты-продолжения
    ** UTF-8 не увеличивают столбец, чтобы кириллица считалась по символам.
    **
    ** The line and column are counted on from the last computed position.
    ** If an offset to the left of the cache is requested, counting starts over.
    ** Line feeds are found with memchr (vectorized), and characters are counted only
    ** in the tail after the last line feed: the validator and DomBuilder
    ** ask for the position of every element, and a byte loop with a branch
    ** over the whole input was a noticeable part of the parsing time. UTF-8
    ** continuation bytes do not increase the column, so that Cyrillic is counted by characters.
    */
    ParsePosition XmlScanner::computePosition(std::size_t offset) const noexcept
    {
        const char* data = _input.data();
        std::size_t lineStart = 0;
        bool newLine = false;

        if (offset > _input.size())
            offset = _input.size();
        if (offset < _cache.offset)
            _cache = ParsePosition{0, 1, 1};
        lineStart = _cache.offset;
        while (lineStart < offset)
        {
            const void* found = std::memchr(data + lineStart, '\n', offset - lineStart);
            if (found == nullptr)
                break;
            ++_cache.line;
            newLine = true;
            lineStart = static_cast<std::size_t>(static_cast<const char*>(found) - data) + 1;
        }
        if (newLine)
            _cache.column = 1;
        if (lineStart < offset)
            _cache.column += countCharacters(data + lineStart, offset - lineStart);
        _cache.offset = offset;
        return _cache;
    }

    // Якорь для текста сущности: позиция ссылки и поясняющий префикс
    // The anchor for an entity text: the reference position and an explanatory prefix
    void XmlScanner::setErrorAnchor(ParsePosition anchor, std::string context)
    {
        _anchor = anchor;
        _context = std::move(context);
    }

    // Срез до первого символа из набора или до конца ввода
    // A slice up to the first character of the set or to the end of input
    std::string_view XmlScanner::readUntilAny(std::string_view symbols) noexcept
    {
        std::size_t found = utils::findAny(_input, _offset, symbols);

        if (found == std::string_view::npos)
            found = _input.size();
        std::string_view content = _input.substr(_offset, found - _offset);
        _offset = found;
        return content;
    }

    // Произвольный срез входа; границы за концом ввода обрезаются (пустой срез, а не исключение)
    // An arbitrary input slice; bounds past the end of input are clipped (an empty slice, not an exception)
    std::string_view XmlScanner::slice(std::size_t begin, std::size_t end) const noexcept
    {
        if (begin > _input.size())
            begin = _input.size();
        if (end < begin)
            end = begin;
        return _input.substr(begin, end - begin);
    }
}
