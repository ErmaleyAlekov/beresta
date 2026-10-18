#ifndef CAPI_HELPERS_HPP
#define CAPI_HELPERS_HPP

#include "capi/beresta_c.h"

#include <cstdint>
#include <string>

namespace support
{
    // Строка от Си-интерфейса -> std::string с обязательным освобождением
    // A string from the C interface -> std::string with a mandatory free
    std::string takeCapiString(char* text);

    // Разбор строки через Си-интерфейс (nullptr при ошибке)
    // Parses a string through the C interface (nullptr on error)
    beresta_document parseCapi(const char* xml, std::uint32_t flags = BERESTA_PARSE_DEFAULT);
}

#endif
