#ifndef UTF8_HPP
#define UTF8_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace beresta::utils
{
    // Допустим ли код символа в XML 1.0 (диапазоны Char из спецификации)
    // Whether a character code is allowed in XML 1.0 (the Char ranges of the specification)
    bool isValidXmlCodePoint(std::uint32_t codePoint) noexcept;

    // Кодирует код символа Unicode в UTF-8 и дописывает байты в конец строки
    // Encodes a Unicode character code to UTF-8 and appends the bytes to the end of the string
    void appendUtf8(std::string& output, std::uint32_t codePoint);

    /*
    ** Декодирует один символ UTF-8, начиная с index. При успехе сдвигает
    ** index за символ и возвращает true. Отвергает обрывы, «длинные» формы
    ** (overlong), суррогаты и коды больше U+10FFFF.
    **
    ** Decodes one UTF-8 character starting at index. On success moves
    ** index past the character and returns true. Rejects truncations, overlong
    ** forms, surrogates and codes above U+10FFFF.
    */
    bool decodeUtf8Char(std::string_view text, std::size_t& index,
        std::uint32_t& codePoint) noexcept;

    // Смещение первого некорректного байта или запрещённого в XML символа; npos — всё верно
    // The offset of the first invalid byte or a character forbidden in XML; npos — everything is valid
    std::size_t findInvalidXmlChar(std::string_view text) noexcept;

    // Количество символов (кодовых точек) в строке UTF-8
    // The number of characters (code points) in a UTF-8 string
    std::size_t countCodePoints(std::string_view text) noexcept;

    // Код символа в начале строки (0, если строка пуста или некорректна)
    // The code of the character at the start of the string (0 if the string is empty or invalid)
    std::uint32_t firstCodePoint(std::string_view text) noexcept;
}

#endif
