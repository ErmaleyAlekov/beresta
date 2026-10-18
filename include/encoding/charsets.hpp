#ifndef CHARSETS_HPP
#define CHARSETS_HPP

#include "encoding/charset.hpp"

#include <cstdint>

namespace beresta::encoding
{
    // UTF-8: декодирование — проверка корректности и копирование без изменений
    // UTF-8: decoding is a validity check and a copy without changes
    class Utf8Charset : public Charset
    {
    public:
        std::string_view getName() const noexcept override;
        std::string_view getByteOrderMark() const noexcept override;
        std::size_t decode(std::string_view bytes, std::string& output) const override;
        std::size_t encode(std::string_view utf8, std::string& output,
            bool useCharacterReferences) const override;
    };

    /*
    ** UTF-16 с заданным порядком байтов. Суррогатные пары собираются
    ** в один символ; одиночный суррогат — ошибка.
    **
    ** UTF-16 with the given byte order. Surrogate pairs are combined
    ** into one character; a lone surrogate is an error.
    */
    class Utf16Charset : public Charset
    {
    public:
        Utf16Charset(std::string_view name, bool bigEndian, bool writeBom) noexcept;

        std::string_view getName() const noexcept override;
        std::string_view getByteOrderMark() const noexcept override;
        std::size_t decode(std::string_view bytes, std::string& output) const override;
        std::size_t encode(std::string_view utf8, std::string& output,
            bool useCharacterReferences) const override;

    private:
        // Читает 16-битное слово с позиции index с учётом порядка байтов
        // Reads a 16-bit word at position index taking the byte order into account
        std::uint32_t readUnit(std::string_view bytes, std::size_t index) const noexcept;

        // Дописывает 16-битное слово с учётом порядка байтов
        // Appends a 16-bit word taking the byte order into account
        void writeUnit(std::string& output, std::uint32_t unit) const;

        std::string_view _name;
        bool _bigEndian;
        bool _writeBom;
    };

    /*
    ** Однобайтовая кодировка: байты 0x00-0x7F — ASCII, а 0x80-0xFF
    ** берутся из таблицы на 128 кодов Unicode (0 — байт не определён).
    ** table == nullptr означает тождественную таблицу (ISO-8859-1);
    ** asciiOnly — байты 0x80-0xFF вообще запрещены (US-ASCII).
    **
    ** A single-byte encoding: bytes 0x00-0x7F are ASCII, and 0x80-0xFF
    ** are taken from a table of 128 Unicode codes (0 — the byte is undefined).
    ** table == nullptr means the identity table (ISO-8859-1);
    ** asciiOnly — bytes 0x80-0xFF are forbidden altogether (US-ASCII).
    */
    class SingleByteCharset : public Charset
    {
    public:
        SingleByteCharset(std::string_view name, const std::uint16_t* table,
            bool asciiOnly) noexcept;

        std::string_view getName() const noexcept override;
        std::string_view getByteOrderMark() const noexcept override;
        std::size_t decode(std::string_view bytes, std::string& output) const override;
        std::size_t encode(std::string_view utf8, std::string& output,
            bool useCharacterReferences) const override;

    private:
        // Код Unicode для старшего байта (0 — не определён)
        // The Unicode code for a high byte (0 — undefined)
        std::uint32_t highByteToCode(unsigned char byte) const noexcept;

        // Байт для кода Unicode >= 0x80 или -1, если символа нет в кодировке
        // The byte for a Unicode code >= 0x80 or -1 if the character is not in the encoding
        int codeToHighByte(std::uint32_t codePoint) const noexcept;

        std::string_view _name;
        const std::uint16_t* _table;
        bool _asciiOnly;
    };

    // Дописывает символьную ссылку &#N; для непредставимого символа
    // Appends the &#N; character reference for an unrepresentable character
    void appendCharacterReference(std::string& output, std::uint32_t codePoint);
}

#endif
