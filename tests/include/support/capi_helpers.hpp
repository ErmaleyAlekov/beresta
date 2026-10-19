#ifndef CAPI_HELPERS_HPP
#define CAPI_HELPERS_HPP

#include "capi/beresta_c.h"

#include <cstdint>
#include <string>
#include <vector>

namespace support
{
    // Строка от Си-интерфейса -> std::string с обязательным освобождением
    // A string from the C interface -> std::string with a mandatory free
    std::string takeCapiString(char* text);

    // Разбор строки через Си-интерфейс (nullptr при ошибке)
    // Parses a string through the C interface (nullptr on error)
    beresta_document parseCapi(const char* xml, std::uint32_t flags = BERESTA_PARSE_DEFAULT);

    // Схема из строки через Си-интерфейс (nullptr при ошибке)
    // A schema from a string through the C interface (nullptr on error)
    beresta_schema loadCapiSchema(const char* xsd);

    // Список нарушений -> строки "строка:столбец: текст"; список освобождается
    // A list of violations -> "line:column: text" strings; the list is freed
    std::vector<std::string> takeCapiDiagnostics(beresta_diagnostics diagnostics);

    /*
    ** Общая пара «схема заказа и документ с двумя нарушениями» для тестов
    ** потоковой проверки и многопоточности: у второго item неверное qty,
    ** у третьего нет обязательного qty.
    **
    ** The common pair "an order schema and a document with two violations" for
    ** the streaming validation and multithreading tests: the second item has a
    ** wrong qty, the third one lacks the required qty.
    */
    extern const char* const ORDER_XSD;
    extern const char* const ORDER_XML;
}

#endif
