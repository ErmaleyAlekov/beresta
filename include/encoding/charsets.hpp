#ifndef CHARSETS_HPP
#define CHARSETS_HPP

#include "encoding/charset.hpp"

#include <cstdint>

namespace beresta::encoding
{
    // UTF-8: декодирование — проверка корректности и копирование без изменений
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
        std::uint32_t readUnit(std::string_view bytes, std::size_t index) const noexcept;

        // Дописывает 16-битное слово с учётом порядка байтов
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
        std::uint32_t highByteToCode(unsigned char byte) const noexcept;

        // Байт для кода Unicode >= 0x80 или -1, если символа нет в кодировке
        int codeToHighByte(std::uint32_t codePoint) const noexcept;

        std::string_view _name;
        const std::uint16_t* _table;
        bool _asciiOnly;
    };

    // Дописывает символьную ссылку &#N; для непредставимого символа
    void appendCharacterReference(std::string& output, std::uint32_t codePoint);
}

#endif
