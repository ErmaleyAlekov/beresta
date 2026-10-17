#ifndef UTF8_HPP
#define UTF8_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace beresta::utils
{
    // Допустим ли код символа в XML 1.0 (диапазоны Char из спецификации)
    bool isValidXmlCodePoint(std::uint32_t codePoint) noexcept;

    // Кодирует код символа Unicode в UTF-8 и дописывает байты в конец строки
    void appendUtf8(std::string& output, std::uint32_t codePoint);

    /*
    ** Декодирует один символ UTF-8, начиная с index. При успехе сдвигает
    ** index за символ и возвращает true. Отвергает обрывы, «длинные» формы
    ** (overlong), суррогаты и коды больше U+10FFFF.
    */
    bool decodeUtf8Char(std::string_view text, std::size_t& index,
        std::uint32_t& codePoint) noexcept;

    // Смещение первого некорректного байта или запрещённого в XML символа; npos — всё верно
    std::size_t findInvalidXmlChar(std::string_view text) noexcept;

    // Количество символов (кодовых точек) в строке UTF-8
    std::size_t countCodePoints(std::string_view text) noexcept;

    // Код символа в начале строки (0, если строка пуста или некорректна)
    std::uint32_t firstCodePoint(std::string_view text) noexcept;
}

#endif
