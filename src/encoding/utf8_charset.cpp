#include "encoding/charsets.hpp"
#include "utils/utf8.hpp"

namespace beresta::encoding
{
    // Каноническое имя
    // The canonical name
    std::string_view Utf8Charset::getName() const noexcept
    {
        return "UTF-8";
    }

    // UTF-8 записывается без BOM: так принято в XML и в Unix-мире
    // UTF-8 is written without a BOM: this is the custom in XML and in the Unix world
    std::string_view Utf8Charset::getByteOrderMark() const noexcept
    {
        return "";
    }

    // Проверяем каждую последовательность; корректные байты копируются как есть
    // Each sequence is checked; valid bytes are copied as is
    std::size_t Utf8Charset::decode(std::string_view bytes, std::string& output) const
    {
        std::size_t index = 0;
        std::uint32_t codePoint = 0;

        while (index < bytes.size())
        {
            std::size_t start = index;
            if (!utils::decodeUtf8Char(bytes, index, codePoint))
                return start;
        }
        output.append(bytes);
        return CHARSET_OK;
    }

    // Внутреннее представление уже UTF-8 — кодирование сводится к копированию
    // The internal representation is already UTF-8 — encoding comes down to copying
    std::size_t Utf8Charset::encode(std::string_view utf8, std::string& output,
        bool useCharacterReferences) const
    {
        (void)useCharacterReferences;
        output.append(utf8);
        return CHARSET_OK;
    }

    // Ссылка вида &#1046; — десятичная запись понятна любому XML-парсеру
    // A reference like &#1046; — the decimal form is understood by any XML parser
    void appendCharacterReference(std::string& output, std::uint32_t codePoint)
    {
        output += "&#";
        output += std::to_string(codePoint);
        output += ';';
    }
}
