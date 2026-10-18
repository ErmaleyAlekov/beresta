#include "utils/utf8.hpp"

namespace beresta::utils
{
    /*
    ** Правило Char из XML 1.0:
    ** #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF].
    ** Суррогаты (#xD800-#xDFFF) и большинство управляющих символов запрещены.
    **
    ** The Char rule of XML 1.0:
    ** #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF].
    ** Surrogates (#xD800-#xDFFF) and most control characters are forbidden.
    */
    bool isValidXmlCodePoint(std::uint32_t codePoint) noexcept
    {
        if (codePoint == 0x9 || codePoint == 0xA || codePoint == 0xD)
            return true;
        if (codePoint >= 0x20 && codePoint <= 0xD7FF)
            return true;
        if (codePoint >= 0xE000 && codePoint <= 0xFFFD)
            return true;
        return codePoint >= 0x10000 && codePoint <= 0x10FFFF;
    }

    /*
    ** Классическое кодирование UTF-8:
    ** до 0x7F — 1 байт, до 0x7FF — 2 байта, до 0xFFFF — 3, иначе 4.
    ** Старшие биты первого байта задают длину, остальные байты имеют вид 10xxxxxx.
    **
    ** Classic UTF-8 encoding:
    ** up to 0x7F — 1 byte, up to 0x7FF — 2 bytes, up to 0xFFFF — 3, otherwise 4.
    ** The high bits of the first byte give the length, the other bytes have the form 10xxxxxx.
    */
    void appendUtf8(std::string& output, std::uint32_t codePoint)
    {
        if (codePoint < 0x80)
        {
            output += static_cast<char>(codePoint);
            return;
        }
        if (codePoint < 0x800)
        {
            output += static_cast<char>(0xC0 | (codePoint >> 6));
            output += static_cast<char>(0x80 | (codePoint & 0x3F));
            return;
        }
        if (codePoint < 0x10000)
        {
            output += static_cast<char>(0xE0 | (codePoint >> 12));
            output += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
            output += static_cast<char>(0x80 | (codePoint & 0x3F));
            return;
        }
        output += static_cast<char>(0xF0 | (codePoint >> 18));
        output += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
        output += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        output += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
}
