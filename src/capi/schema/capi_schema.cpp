#include "capi/internal/capi_support.hpp"
#include "parser/resource_resolver.hpp"
#include "xsd/schema_validator.hpp"

// Загрузка из байтов XSD (кодировка определяется так же, как у документа)
extern "C" BERESTA_API int beresta_schema_load(const void* data, int64_t size, beresta_schema* out)
{
    BERESTA_CAPI_TRY
        if (out == nullptr || size < 0 || (data == nullptr && size != 0))
            throw beresta::capi::InvalidArgumentException("data/size/out: неверные аргументы");
        std::string_view text(static_cast<const char*>(data), static_cast<std::size_t>(size));

        *out = new beresta::capi::SchemaHandle{beresta::xsd::Schema::fromString(text)};
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

/*
** Загрузка из файла. xs:include / xs:import разрешаются относительно
** каталога схемы через FileResourceResolver; он живёт только на время
** загрузки — готовая схема уже не обращается к файловой системе.
*/
extern "C" BERESTA_API int beresta_schema_load_file(const char* path, beresta_schema* out)
{
    BERESTA_CAPI_TRY
        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException("out: передан NULL");
        std::filesystem::path file = std::filesystem::absolute(beresta::capi::pathFromUtf8(path));
        beresta::FileResourceResolver resolver(file.parent_path());
        beresta::xsd::SchemaLoadOptions options;

        options.resolver = &resolver;
        *out = new beresta::capi::SchemaHandle{beresta::xsd::Schema::fromFile(file, options)};
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

// shared_ptr внутри: документы, разобранные с этой схемой, не зависят от дескриптора
extern "C" BERESTA_API void beresta_schema_free(beresta_schema schema)
{
    delete static_cast<beresta::capi::SchemaHandle*>(schema);
}

// Проверка: все нарушения собираются в список, документ не меняется
extern "C" BERESTA_API int beresta_schema_validate(beresta_schema schema, beresta_document document,
    beresta_diagnostics* out)
{
    BERESTA_CAPI_TRY
        const beresta::capi::SchemaHandle& handle = beresta::capi::requireHandle<beresta::capi::SchemaHandle>(schema, "schema");
        const beresta::Document& source = beresta::capi::toDocument(document);

        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException("out: передан NULL");
        beresta::xsd::SchemaValidator validator(handle.schema);
        *out = new beresta::capi::DiagnosticsHandle{validator.validate(source)};
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

// Список нарушений создаёт beresta_schema_validate — освобождение рядом, в том же модуле
extern "C" BERESTA_API void beresta_diagnostics_free(beresta_diagnostics diagnostics)
{
    delete static_cast<beresta::capi::DiagnosticsHandle*>(diagnostics);
}
