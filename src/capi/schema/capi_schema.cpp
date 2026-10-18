#include "capi/internal/capi_support.hpp"
#include "i18n/i18n.hpp"
#include "parser/resource_resolver.hpp"
#include "xsd/schema_validator.hpp"

// Загрузка из байтов XSD (кодировка определяется так же, как у документа)
// Loading from XSD bytes (the encoding is detected the same way as for a document)
extern "C" BERESTA_API int beresta_schema_load(const void* data, int64_t size, beresta_schema* out)
{
    BERESTA_CAPI_TRY
        if (out == nullptr || size < 0 || (data == nullptr && size != 0))
            throw beresta::capi::InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::ARGUMENT_BAD_BUFFER, {"data/size/out"}));
        std::string_view text(static_cast<const char*>(data), static_cast<std::size_t>(size));

        *out = new beresta::capi::SchemaHandle{beresta::xsd::Schema::fromString(text)};
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

/*
** Загрузка из файла. xs:include / xs:import разрешаются относительно
** каталога схемы через FileResourceResolver; он живёт только на время
** загрузки — готовая схема уже не обращается к файловой системе.
**
** Loading from a file. xs:include / xs:import are resolved relative to
** the schema directory through FileResourceResolver; it lives only during
** loading — the ready schema no longer accesses the file system.
*/
extern "C" BERESTA_API int beresta_schema_load_file(const char* path, beresta_schema* out)
{
    BERESTA_CAPI_TRY
        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::ARGUMENT_NULL, {"out"}));
        std::filesystem::path file = std::filesystem::absolute(beresta::capi::pathFromUtf8(path));
        beresta::FileResourceResolver resolver(file.parent_path());
        beresta::xsd::SchemaLoadOptions options;

        options.resolver = &resolver;
        *out = new beresta::capi::SchemaHandle{beresta::xsd::Schema::fromFile(file, options)};
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

// shared_ptr внутри: документы, разобранные с этой схемой, не зависят от дескриптора
// shared_ptr inside: documents parsed with this schema do not depend on the handle
extern "C" BERESTA_API void beresta_schema_free(beresta_schema schema)
{
    delete static_cast<beresta::capi::SchemaHandle*>(schema);
}

// Проверка: все нарушения собираются в список, документ не меняется
// Validation: all violations are collected into a list, the document does not change
extern "C" BERESTA_API int beresta_schema_validate(beresta_schema schema, beresta_document document,
    beresta_diagnostics* out)
{
    BERESTA_CAPI_TRY
        const beresta::capi::SchemaHandle& handle = beresta::capi::requireHandle<beresta::capi::SchemaHandle>(schema, "schema");
        const beresta::Document& source = beresta::capi::toDocument(document);

        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::ARGUMENT_NULL, {"out"}));
        beresta::xsd::SchemaValidator validator(handle.schema);
        *out = new beresta::capi::DiagnosticsHandle{validator.validate(source)};
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

// Список нарушений создаёт beresta_schema_validate — освобождение рядом, в том же модуле
// beresta_schema_validate creates the list of violations — it is freed nearby, in the same module
extern "C" BERESTA_API void beresta_diagnostics_free(beresta_diagnostics diagnostics)
{
    delete static_cast<beresta::capi::DiagnosticsHandle*>(diagnostics);
}
