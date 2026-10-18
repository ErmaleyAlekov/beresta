#include "encoding/charsets.hpp"

namespace beresta::encoding
{
    // Прямой поиск в таблице; для Latin-1 код совпадает с байтом
    // A direct table lookup; for Latin-1 the code equals the byte
    std::uint32_t SingleByteCharset::highByteToCode(unsigned char byte) const noexcept
    {
        if (_asciiOnly)
            return 0;
        if (_table == nullptr)
            return byte;
        return _table[byte - 0x80];
    }

    /*
    ** Обратный поиск — линейный по 128 элементам. Это быстрее и проще
    ** хеш-таблицы для такого размера и не требует инициализации.
    **
    ** The reverse lookup is linear over 128 items. For this size it is faster
    ** and simpler than a hash table and needs no initialization.
    */
    int SingleByteCharset::codeToHighByte(std::uint32_t codePoint) const noexcept
    {
        if (_asciiOnly)
            return -1;
        if (_table == nullptr)
        {
            if (codePoint <= 0xFF)
                return static_cast<int>(codePoint);
            return -1;
        }
        for (int index = 0; index < 128; ++index)
        {
            if (_table[index] == codePoint)
                return 0x80 + index;
        }
        return -1;
    }
}
