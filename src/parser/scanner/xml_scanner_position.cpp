#include "parser/xml_scanner.hpp"

#include "utils/string_utils.hpp"

#include <utility>

namespace beresta::detail
{
    /*
    ** Строка и столбец досчитываются от последней вычисленной позиции.
    ** Если запрошено смещение левее кэша — счёт начинается заново.
    ** Байты-продолжения UTF-8 (вида 10xxxxxx) не увеличивают столбец,
    ** чтобы кириллица считалась по символам, а не по байтам.
    **
    ** The line and column are counted on from the last computed position.
    ** If an offset to the left of the cache is requested, counting starts over.
    ** UTF-8 continuation bytes (of the form 10xxxxxx) do not increase the column,
    ** so that Cyrillic is counted by characters, not by bytes.
    */
    ParsePosition XmlScanner::computePosition(std::size_t offset) const noexcept
    {
        if (offset > _input.size())
            offset = _input.size();
        if (offset < _cache.offset)
            _cache = ParsePosition{0, 1, 1};
        for (std::size_t index = _cache.offset; index < offset; ++index)
        {
            unsigned char code = static_cast<unsigned char>(_input[index]);
            if (code == '\n')
            {
                ++_cache.line;
                _cache.column = 1;
            }
            else if ((code & 0xC0) != 0x80)
                ++_cache.column;
        }
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
