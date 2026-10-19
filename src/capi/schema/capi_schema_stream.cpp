#include "capi/internal/capi_support.hpp"
#include "i18n/i18n.hpp"
#include "parser/resource_resolver.hpp"
#include "sax/sax_parser.hpp"
#include "xsd/validating_handler.hpp"

namespace
{
    /*
    ** Общая проверка аргументов: без схемы проверять не по чему, без out
    ** результат некуда записать. Возвращается сама схема (shared_ptr):
    ** валидатор держит её, пока идёт разбор.
    **
    ** The common argument check: without a schema there is nothing to validate
    ** against, without out there is nowhere to write the result. The schema itself
    ** is returned (shared_ptr): the validator holds it while parsing goes on.
    */
    std::shared_ptr<const beresta::xsd::Schema> requireSchema(beresta_schema schema, const void* out)
    {
        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::ARGUMENT_NULL, {"out"}));
        return beresta::capi::requireHandle<beresta::capi::SchemaHandle>(schema, "schema").schema;
    }

    /*
    ** Источник для разбора: байты в памяти или файл. Структура вместо
    ** двух почти одинаковых функций (DRY): цепочка «валидатор -> SAX» одна.
    **
    ** The source for parsing: bytes in memory or a file. A structure instead
    ** of two almost identical functions (DRY): the "validator -> SAX" chain is one.
    */
    struct StreamSource
    {
        std::string_view bytes;
        const std::filesystem::path* file;
    };

    /*
    ** ValidatingHandler без следующего обработчика (downstream == nullptr):
    ** события SAX уходят только в валидатор, дерево не строится. Схема
    ** в ParserOptions не кладётся — иначе конвейер SAX сам отправил бы
    ** нарушения в errorHandler (или бросил бы первое), а здесь нужен весь
    ** список. Настройки валидатора согласуются с флагами разбора так же,
    ** как у разбора в дерево, поэтому нарушения совпадают с beresta_schema_validate.
    **
    ** A ValidatingHandler without a next handler (downstream == nullptr):
    ** SAX events go only to the validator, no tree is built. The schema is
    ** not put into ParserOptions — otherwise the SAX pipeline would itself send
    ** the violations to errorHandler (or throw the first one), while the whole
    ** list is needed here. The validator settings are matched with the parsing
    ** flags the same way as for parsing into a tree, so the violations are the
    ** same as from beresta_schema_validate.
    */
    beresta_diagnostics validateStream(const StreamSource& source, const beresta::ParserOptions& options,
        std::shared_ptr<const beresta::xsd::Schema> schema)
    {
        beresta::xsd::ValidatingHandler validator(std::move(schema), nullptr,
            beresta::xsd::validationSettingsFor(options, beresta::xsd::IdentityEvaluation::STREAMING));
        beresta::SaxParser parser(options);

        if (source.file != nullptr)
            parser.parseFile(*source.file, validator);
        else
            parser.parse(source.bytes, validator);
        return new beresta::capi::DiagnosticsHandle{validator.takeDiagnostics()};
    }
}

// Проверка байтов в любой поддерживаемой кодировке
// Validation of bytes in any supported encoding
extern "C" BERESTA_API int beresta_schema_validate_bytes(const void* data, int64_t size, uint32_t flags,
    beresta_schema schema, beresta_diagnostics* out)
{
    BERESTA_CAPI_TRY
        std::shared_ptr<const beresta::xsd::Schema> loaded = requireSchema(schema, out);

        if (size < 0 || (data == nullptr && size != 0))
            throw beresta::capi::InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::ARGUMENT_BAD_BUFFER, {"data/size"}));
        StreamSource source{std::string_view(static_cast<const char*>(data), static_cast<std::size_t>(size)), nullptr};
        *out = validateStream(source, beresta::capi::parserOptions(flags, nullptr), std::move(loaded));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

/*
** Проверка файла. BERESTA_PARSE_FILE_RESOURCES — как у
** beresta_document_parse_file: внешние DTD и сущности только из каталога
** файла и его подкаталогов.
**
** Validation of a file. BERESTA_PARSE_FILE_RESOURCES works as for
** beresta_document_parse_file: external DTDs and entities only from the
** directory of the file and its subdirectories.
*/
extern "C" BERESTA_API int beresta_schema_validate_file(const char* path, uint32_t flags,
    beresta_schema schema, beresta_diagnostics* out)
{
    BERESTA_CAPI_TRY
        std::shared_ptr<const beresta::xsd::Schema> loaded = requireSchema(schema, out);
        std::filesystem::path file = std::filesystem::absolute(beresta::capi::pathFromUtf8(path));
        beresta::ParserOptions options = beresta::capi::parserOptions(flags, nullptr);
        beresta::FileResourceResolver resolver(file.parent_path());

        if ((flags & BERESTA_PARSE_FILE_RESOURCES) != 0)
        {
            options.resolver = &resolver;
            options.loadExternalDtd = true;
        }
        *out = validateStream(StreamSource{std::string_view(), &file}, options, std::move(loaded));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}
