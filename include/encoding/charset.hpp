#ifndef CHARSET_HPP
#define CHARSET_HPP

#include <cstddef>
#include <string>
#include <string_view>

namespace beresta::encoding
{
    // Результат decode()/encode(), означающий успех (иначе — смещение ошибки)
    // The decode()/encode() result meaning success (otherwise — the error offset)
    inline constexpr std::size_t CHARSET_OK = static_cast<std::size_t>(-1);

    /*
    ** Charset — стратегия перекодирования между внешней кодировкой и
    ** внутренним UTF-8 (паттерн Strategy). Все таблицы и алгоритмы
    ** написаны самостоятельно: ни iconv, ни ICU не нужны.
    **
    ** decode: байты внешней кодировки -> UTF-8 (дописывается в output).
    ** encode: UTF-8 -> байты; символ, которого нет в кодировке, при
    **         useCharacterReferences записывается как &#N; (корректно
    **         в тексте и атрибутах XML), иначе кодирование прерывается.
    ** Оба метода возвращают CHARSET_OK или смещение проблемного байта.
    **
    ** Charset — a strategy of transcoding between an external encoding and
    ** the internal UTF-8 (the Strategy pattern). All tables and algorithms
    ** are written by hand: neither iconv nor ICU is needed.
    ** decode: bytes of the external encoding -> UTF-8 (appended to output).
    ** encode: UTF-8 -> bytes; a character missing from the encoding is written,
    ** with useCharacterReferences, as &#N; (valid in XML text and attributes),
    ** otherwise encoding stops.
    ** Both methods return CHARSET_OK or the offset of the problematic byte.
    */
    class Charset
    {
    public:
        virtual ~Charset();

        // Каноническое имя кодировки (как в XML-декларации)
        // The canonical encoding name (as in the XML declaration)
        virtual std::string_view getName() const noexcept = 0;

        // Метка порядка байтов, которую нужно записать в начало файла ("" — не нужна)
        // The byte order mark to write at the start of the file ("" — not needed)
        virtual std::string_view getByteOrderMark() const noexcept = 0;

        virtual std::size_t decode(std::string_view bytes, std::string& output) const = 0;

        virtual std::size_t encode(std::string_view utf8, std::string& output,
            bool useCharacterReferences) const = 0;
    };

    /*
    ** Поиск кодировки по имени без учёта регистра, с псевдонимами
    ** (например, "cp1251" = "windows-1251"). nullptr — кодировка не поддерживается.
    ** Поддерживаются: UTF-8, UTF-16 (LE/BE), ISO-8859-1, US-ASCII,
    ** windows-1251, windows-1252, KOI8-R, IBM866.
    **
    ** Looks up an encoding by name, case-insensitively, with aliases
    ** (for example, "cp1251" = "windows-1251"). nullptr — the encoding is not supported.
    ** Supported: UTF-8, UTF-16 (LE/BE), ISO-8859-1, US-ASCII,
    ** windows-1251, windows-1252, KOI8-R, IBM866.
    */
    const Charset* findCharset(std::string_view name) noexcept;

    // Внутренняя кодировка библиотеки
    // The internal encoding of the library
    const Charset& utf8Charset() noexcept;
}

#endif
