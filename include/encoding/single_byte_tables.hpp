#ifndef SINGLE_BYTE_TABLES_HPP
#define SINGLE_BYTE_TABLES_HPP

#include <cstdint>

namespace beresta::encoding
{
    /*
    ** Таблицы старшей половины однобайтовых кодировок: элемент i — код
    ** Unicode для байта 0x80 + i; 0 означает «байт не определён».
    ** Данные лежат в src/encoding/tables/ (по одной таблице на файл).
    **
    ** Tables of the upper half of single-byte encodings: element i is the Unicode
    ** code for byte 0x80 + i; 0 means "the byte is undefined".
    ** The data lives in src/encoding/tables/ (one table per file).
    */
    extern const std::uint16_t WINDOWS_1251_TABLE[128];
    extern const std::uint16_t WINDOWS_1252_TABLE[128];
    extern const std::uint16_t KOI8_R_TABLE[128];
    extern const std::uint16_t IBM866_TABLE[128];
}

#endif
