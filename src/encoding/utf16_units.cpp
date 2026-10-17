#include "encoding/charsets.hpp"

namespace beresta::encoding
{
    // Слово из двух байтов: порядок определяется вариантом кодировки (BE / LE)
    std::uint32_t Utf16Charset::readUnit(std::string_view bytes, std::size_t index) const noexcept
    {
        std::uint32_t first = static_cast<unsigned char>(bytes[index]);
        std::uint32_t second = static_cast<unsigned char>(bytes[index + 1]);

        if (_bigEndian)
            return (first << 8) | second;
        return (second << 8) | first;
    }

    // Запись слова в выбранном порядке байтов
    void Utf16Charset::writeUnit(std::string& output, std::uint32_t unit) const
    {
        char high = static_cast<char>((unit >> 8) & 0xFF);
        char low = static_cast<char>(unit & 0xFF);

        if (_bigEndian)
        {
            output += high;
            output += low;
            return;
        }
        output += low;
        output += high;
    }
}
