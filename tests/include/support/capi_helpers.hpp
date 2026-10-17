#ifndef CAPI_HELPERS_HPP
#define CAPI_HELPERS_HPP

#include "capi/beresta_c.h"

#include <cstdint>
#include <string>

namespace support
{
    // Строка от Си-интерфейса -> std::string с обязательным освобождением
    std::string takeCapiString(char* text);

    // Разбор строки через Си-интерфейс (nullptr при ошибке)
    beresta_document parseCapi(const char* xml, std::uint32_t flags = BERESTA_PARSE_DEFAULT);
}

#endif
