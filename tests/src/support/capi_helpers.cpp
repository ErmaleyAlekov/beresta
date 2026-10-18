#include "support/capi_helpers.hpp"

#include <cstring>

namespace support
{
    // NULL превращается в пустую строку; память освобождается всегда
    // NULL becomes an empty string; the memory is always freed
    std::string takeCapiString(char* text)
    {
        std::string result;

        if (text != nullptr)
            result = text;
        beresta_string_free(text);
        return result;
    }

    // Обёртка над beresta_document_parse для строк, завершённых нулём
    // A wrapper over beresta_document_parse for null-terminated strings
    beresta_document parseCapi(const char* xml, std::uint32_t flags)
    {
        beresta_document document = nullptr;

        beresta_document_parse(xml, static_cast<int64_t>(std::strlen(xml)), flags, nullptr, &document);
        return document;
    }
}
