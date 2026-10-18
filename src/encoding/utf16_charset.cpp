#include "encoding/charsets.hpp"
#include "utils/utf8.hpp"

namespace beresta::encoding
{
    // Параметры варианта UTF-16 задаются при регистрации кодировки
    // The parameters of a UTF-16 variant are set when the encoding is registered
    Utf16Charset::Utf16Charset(std::string_view name, bool bigEndian, bool writeBom) noexcept
        : _name(name), _bigEndian(bigEndian), _writeBom(writeBom)
    {
    }

    // Каноническое имя
    // The canonical name
    std::string_view Utf16Charset::getName() const noexcept
    {
        return _name;
    }

    // Обобщённый "UTF-16" пишется с BOM, чтобы читатель узнал порядок байтов
    // The generic "UTF-16" is written with a BOM so that the reader knows the byte order
    std::string_view Utf16Charset::getByteOrderMark() const noexcept
    {
        if (!_writeBom)
            return "";
        if (_bigEndian)
            return "\xFE\xFF";
        return "\xFF\xFE";
    }

    /*
    ** Чтение по 16-битным словам. Старший суррогат (D800-DBFF) обязан
    ** сопровождаться младшим (DC00-DFFF): вместе они дают символ > U+FFFF.
    **
    ** Reading by 16-bit words. A high surrogate (D800-DBFF) must be
    ** followed by a low one (DC00-DFFF): together they give a character > U+FFFF.
    */
    std::size_t Utf16Charset::decode(std::string_view bytes, std::string& output) const
    {
        std::size_t index = 0;

        if (bytes.size() % 2 != 0)
            return bytes.size() - 1;
        while (index < bytes.size())
        {
            std::uint32_t unit = readUnit(bytes, index);
            std::size_t start = index;
            index += 2;
            if (unit >= 0xDC00 && unit <= 0xDFFF)
                return start;
            if (unit >= 0xD800 && unit <= 0xDBFF)
            {
                if (index >= bytes.size() || readUnit(bytes, index) < 0xDC00
                    || readUnit(bytes, index) > 0xDFFF)
                    return start;
                unit = 0x10000 + ((unit - 0xD800) << 10) + (readUnit(bytes, index) - 0xDC00);
                index += 2;
            }
            utils::appendUtf8(output, unit);
        }
        return CHARSET_OK;
    }

    // Символы > U+FFFF кодируются суррогатной парой; UTF-16 представляет любой символ
    // Characters > U+FFFF are encoded with a surrogate pair; UTF-16 represents any character
    std::size_t Utf16Charset::encode(std::string_view utf8, std::string& output,
        bool useCharacterReferences) const
    {
        std::size_t index = 0;
        std::uint32_t codePoint = 0;

        (void)useCharacterReferences;
        while (index < utf8.size())
        {
            std::size_t start = index;
            if (!utils::decodeUtf8Char(utf8, index, codePoint))
                return start;
            if (codePoint < 0x10000)
            {
                writeUnit(output, codePoint);
                continue;
            }
            writeUnit(output, 0xD800 + ((codePoint - 0x10000) >> 10));
            writeUnit(output, 0xDC00 + ((codePoint - 0x10000) & 0x3FF));
        }
        return CHARSET_OK;
    }
}
