#include "utils/utf8.hpp"

namespace beresta::utils
{
    /*
    ** Разбор ведущего байта: старшие биты задают длину последовательности,
    ** оставшиеся — старшие биты кода. minimum — наименьший код, допустимый
    ** для этой длины (меньшие коды — запрещённая «длинная» форма).
    **
    ** Parses the leading byte: the high bits give the sequence length,
    ** the remaining ones are the high bits of the code. minimum is the smallest code allowed
    ** for this length (smaller codes are a forbidden overlong form).
    */
    static bool readLeadByte(unsigned char lead, std::size_t& length,
        std::uint32_t& codePoint, std::uint32_t& minimum) noexcept
    {
        if ((lead & 0xE0) == 0xC0)
        {
            length = 2;
            codePoint = lead & 0x1F;
            minimum = 0x80;
            return true;
        }
        if ((lead & 0xF0) == 0xE0)
        {
            length = 3;
            codePoint = lead & 0x0F;
            minimum = 0x800;
            return true;
        }
        if ((lead & 0xF8) != 0xF0)
            return false;
        length = 4;
        codePoint = lead & 0x07;
        minimum = 0x10000;
        return true;
    }

    // Ведущий байт + байты-продолжения вида 10xxxxxx; затем проверка диапазона
    // The leading byte + continuation bytes of the form 10xxxxxx; then the range check
    bool decodeUtf8Char(std::string_view text, std::size_t& index,
        std::uint32_t& codePoint) noexcept
    {
        unsigned char lead = static_cast<unsigned char>(text[index]);
        std::size_t length = 1;
        std::uint32_t minimum = 0;

        if (lead < 0x80)
        {
            codePoint = lead;
            ++index;
            return true;
        }
        if (!readLeadByte(lead, length, codePoint, minimum) || index + length > text.size())
            return false;
        for (std::size_t offset = 1; offset < length; ++offset)
        {
            unsigned char next = static_cast<unsigned char>(text[index + offset]);
            if ((next & 0xC0) != 0x80)
                return false;
            codePoint = (codePoint << 6) | (next & 0x3F);
        }
        if (codePoint < minimum || codePoint > 0x10FFFF)
            return false;
        if (codePoint >= 0xD800 && codePoint <= 0xDFFF)
            return false;
        index += length;
        return true;
    }

    /*
    ** Один проход по всему входу: печатные ASCII-символы проверяются
    ** быстрым путём, остальные — полным декодированием и правилом Char.
    **
    ** One pass over the whole input: printable ASCII characters are checked
    ** by the fast path, the rest by full decoding and the Char rule.
    */
    std::size_t findInvalidXmlChar(std::string_view text) noexcept
    {
        std::size_t index = 0;

        while (index < text.size())
        {
            unsigned char byte = static_cast<unsigned char>(text[index]);
            if (byte >= 0x20 && byte < 0x80)
            {
                ++index;
                continue;
            }
            if (byte >= 0xC2 && byte <= 0xDF && index + 1 < text.size()
                && (static_cast<unsigned char>(text[index + 1]) & 0xC0) == 0x80)
            {
                index += 2;
                continue;
            }
            std::size_t start = index;
            std::uint32_t codePoint = 0;
            if (!decodeUtf8Char(text, index, codePoint) || !isValidXmlCodePoint(codePoint))
                return start;
        }
        return std::string_view::npos;
    }
}
