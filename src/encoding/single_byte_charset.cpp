#include "encoding/charsets.hpp"
#include "utils/utf8.hpp"

namespace beresta::encoding
{
    // Однобайтовая кодировка задаётся таблицей старшей половины
    SingleByteCharset::SingleByteCharset(std::string_view name, const std::uint16_t* table,
        bool asciiOnly) noexcept
        : _name(name), _table(table), _asciiOnly(asciiOnly)
    {
    }

    // Каноническое имя
    std::string_view SingleByteCharset::getName() const noexcept
    {
        return _name;
    }

    // Однобайтовые кодировки не используют BOM
    std::string_view SingleByteCharset::getByteOrderMark() const noexcept
    {
        return "";
    }

    // Каждый байт — ровно один символ; неопределённый байт — ошибка
    std::size_t SingleByteCharset::decode(std::string_view bytes, std::string& output) const
    {
        output.reserve(output.size() + bytes.size() * 2);
        for (std::size_t index = 0; index < bytes.size(); ++index)
        {
            unsigned char byte = static_cast<unsigned char>(bytes[index]);
            if (byte < 0x80)
            {
                output += static_cast<char>(byte);
                continue;
            }
            std::uint32_t codePoint = highByteToCode(byte);
            if (codePoint == 0)
                return index;
            utils::appendUtf8(output, codePoint);
        }
        return CHARSET_OK;
    }

    // Символ вне кодировки — символьная ссылка или ошибка (по выбору вызывающего)
    std::size_t SingleByteCharset::encode(std::string_view utf8, std::string& output,
        bool useCharacterReferences) const
    {
        std::size_t index = 0;
        std::uint32_t codePoint = 0;

        while (index < utf8.size())
        {
            std::size_t start = index;
            if (!utils::decodeUtf8Char(utf8, index, codePoint))
                return start;
            int byte = static_cast<int>(codePoint);
            if (codePoint >= 0x80)
                byte = codeToHighByte(codePoint);
            if (byte >= 0)
                output += static_cast<char>(byte);
            else if (useCharacterReferences)
                appendCharacterReference(output, codePoint);
            else
                return start;
        }
        return CHARSET_OK;
    }
}
