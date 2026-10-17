#include "capi/internal/capi_support.hpp"
#include "parser/resource_resolver.hpp"
#include "parser/xml_parser.hpp"

namespace
{
    // Проверка выходного параметра: без него результат некуда записать
    void requireOutput(const void* out)
    {
        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException("out: передан NULL");
    }
}

/*
** Разбор байтов. Кодировку определяет ядро (BOM, XML-декларация), поэтому
** обёртки передают сырые байты — например, содержимое файла или сетевого
** ответа — без перекодирования на своей стороне.
*/
extern "C" BERESTA_API int beresta_document_parse(const void* data, int64_t size, uint32_t flags,
    beresta_schema schema, beresta_document* out)
{
    BERESTA_CAPI_TRY
        requireOutput(out);
        if (size < 0 || (data == nullptr && size != 0))
            throw beresta::capi::InvalidArgumentException("data/size: неверный буфер");
        std::string_view bytes(static_cast<const char*>(data), static_cast<std::size_t>(size));
        beresta::XmlParser parser(beresta::capi::parserOptions(flags, schema));

        *out = beresta::capi::releaseDocument(parser.parse(bytes));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

/*
** Разбор файла. Флаг BERESTA_PARSE_FILE_RESOURCES разрешает внешние DTD,
** сущности и XInclude, но только из каталога самого файла и его
** подкаталогов (FileResourceResolver не выпускает за корень) — защита
** от XXE остаётся в силе даже при включённых внешних ресурсах.
*/
extern "C" BERESTA_API int beresta_document_parse_file(const char* path, uint32_t flags,
    beresta_schema schema, beresta_document* out)
{
    BERESTA_CAPI_TRY
        requireOutput(out);
        std::filesystem::path file = std::filesystem::absolute(beresta::capi::pathFromUtf8(path));
        beresta::ParserOptions options = beresta::capi::parserOptions(flags, schema);
        beresta::FileResourceResolver resolver(file.parent_path());

        if ((flags & BERESTA_PARSE_FILE_RESOURCES) != 0)
        {
            options.resolver = &resolver;
            options.loadExternalDtd = true;
        }
        *out = beresta::capi::releaseDocument(beresta::XmlParser(options).parseFile(file));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

// Запись в файл в выбранной кодировке (символы вне кодировки — как &#N;)
extern "C" BERESTA_API int beresta_document_save(beresta_document document, const char* path,
    uint32_t flags, const char* encoding)
{
    BERESTA_CAPI_TRY
        const beresta::Document& source = beresta::capi::toDocument(document);
        beresta::XmlSerializer serializer(beresta::capi::serializerOptions(flags, encoding));

        serializer.writeFile(source, beresta::capi::pathFromUtf8(path));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

// Поиск по ID; Element* приводится к Node* до void* (единое правило дескрипторов)
extern "C" BERESTA_API beresta_node beresta_document_element_by_id(beresta_document document, const char* id)
{
    BERESTA_CAPI_TRY
        beresta::Document& owner = beresta::capi::toDocument(document);
        beresta::Node* found = owner.getElementById(beresta::capi::requireText(id, "id"));

        return found;
    BERESTA_CAPI_CATCH(nullptr)
}
