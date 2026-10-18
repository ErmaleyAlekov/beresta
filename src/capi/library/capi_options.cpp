#include "capi/internal/capi_support.hpp"
#include "i18n/i18n.hpp"

namespace beresta::capi
{
    /*
    ** Путь из UTF-8. В C++17 конструктор path(std::string) на Windows
    ** трактует байты как ANSI (cp1251 и т.п.), и кириллический путь
    ** превратился бы в мусор. u8path явно говорит: это UTF-8.
    **
    ** A path from UTF-8. In C++17 the path(std::string) constructor on Windows
    ** treats the bytes as ANSI (cp1251 and so on), and a Cyrillic path
    ** would turn into garbage. u8path says explicitly: this is UTF-8.
    */
    std::filesystem::path pathFromUtf8(const char* path)
    {
        return std::filesystem::u8path(requireText(path, "path"));
    }

    // Проверка одного бита маски
    // Checks one bit of the mask
    static bool hasFlag(std::uint32_t flags, std::uint32_t flag) noexcept
    {
        return (flags & flag) != 0;
    }

    /*
    ** Битовая маска -> ParserOptions. Значения по умолчанию ядра
    ** (лимиты, защита от XXE) не трогаются: маска только включает
    ** дополнительные возможности. Ресурсы из файлов включаются отдельно
    ** в beresta_document_parse_file — только там известен каталог.
    **
    ** A bit mask -> ParserOptions. The core defaults (limits, XXE protection)
    ** are not touched: the mask only enables additional features.
    ** File resources are enabled separately in beresta_document_parse_file —
    ** only there the directory is known.
    */
    ParserOptions parserOptions(std::uint32_t flags, void* schema)
    {
        ParserOptions options;

        options.preserveWhitespaceText = hasFlag(flags, BERESTA_PARSE_PRESERVE_WHITESPACE);
        options.discardComments = hasFlag(flags, BERESTA_PARSE_DISCARD_COMMENTS);
        options.coalesceCData = hasFlag(flags, BERESTA_PARSE_COALESCE_CDATA);
        options.validateDtd = hasFlag(flags, BERESTA_PARSE_VALIDATE_DTD);
        options.namespaceAware = !hasFlag(flags, BERESTA_PARSE_NO_NAMESPACES);
        options.processXInclude = hasFlag(flags, BERESTA_PARSE_XINCLUDE);
        if (schema != nullptr)
            options.schema = requireHandle<SchemaHandle>(schema, "schema").schema;
        return options;
    }

    // Битовая маска -> SerializerOptions; encoding == NULL — UTF-8
    // A bit mask -> SerializerOptions; encoding == NULL means UTF-8
    SerializerOptions serializerOptions(std::uint32_t flags, const char* encoding)
    {
        SerializerOptions options;

        options.prettyPrint = hasFlag(flags, BERESTA_WRITE_PRETTY);
        options.forceDeclaration = hasFlag(flags, BERESTA_WRITE_DECLARATION);
        if (encoding != nullptr)
            options.encoding = encoding;
        return options;
    }
}
