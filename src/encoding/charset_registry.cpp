#include "encoding/charsets.hpp"
#include "encoding/single_byte_tables.hpp"
#include "utils/string_utils.hpp"

namespace beresta::encoding
{
    // Виртуальный деструктор интерфейса
    Charset::~Charset()
    {
    }

    // Псевдоним имени кодировки и номер кодировки в реестре
    struct CharsetAlias
    {
        const char* alias;
        int index;
    };

    /*
    ** Реестр (паттерн Registry): единственные экземпляры всех кодировок.
    ** Статические локальные объекты создаются потокобезопасно при первом
    ** обращении и живут до конца программы.
    */
    static const Charset* charsetByIndex(int index) noexcept
    {
        static const Utf8Charset UTF8;
        static const Utf16Charset UTF16("UTF-16", false, true);
        static const Utf16Charset UTF16LE("UTF-16LE", false, false);
        static const Utf16Charset UTF16BE("UTF-16BE", true, false);
        static const SingleByteCharset LATIN1("ISO-8859-1", nullptr, false);
        static const SingleByteCharset ASCII("US-ASCII", nullptr, true);
        static const SingleByteCharset CP1251("windows-1251", WINDOWS_1251_TABLE, false);
        static const SingleByteCharset CP1252("windows-1252", WINDOWS_1252_TABLE, false);
        static const SingleByteCharset KOI8R("KOI8-R", KOI8_R_TABLE, false);
        static const SingleByteCharset CP866("IBM866", IBM866_TABLE, false);
        static const Charset* const ALL[] = {
            &UTF8, &UTF16, &UTF16LE, &UTF16BE, &LATIN1, &ASCII, &CP1251, &CP1252, &KOI8R, &CP866
        };

        return ALL[index];
    }

    // Поиск по таблице псевдонимов без учёта регистра
    const Charset* findCharset(std::string_view name) noexcept
    {
        static const CharsetAlias ALIASES[] = {
            {"utf-8", 0}, {"utf8", 0}, {"utf-16", 1}, {"utf16", 1}, {"utf-16le", 2},
            {"utf-16be", 3}, {"iso-8859-1", 4}, {"latin1", 4}, {"iso_8859-1", 4}, {"l1", 4},
            {"us-ascii", 5}, {"ascii", 5}, {"windows-1251", 6}, {"cp1251", 6}, {"win-1251", 6},
            {"windows-1252", 7}, {"cp1252", 7}, {"koi8-r", 8}, {"koi8r", 8},
            {"ibm866", 9}, {"cp866", 9}, {"866", 9}
        };

        for (const CharsetAlias& entry : ALIASES)
        {
            if (utils::equalsIgnoreCaseAscii(name, entry.alias))
                return charsetByIndex(entry.index);
        }
        return nullptr;
    }

    // Внутренняя кодировка — всегда первая в реестре
    const Charset& utf8Charset() noexcept
    {
        return *charsetByIndex(0);
    }
}
