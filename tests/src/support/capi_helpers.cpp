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

    // Обёртка над beresta_schema_load / A wrapper over beresta_schema_load
    beresta_schema loadCapiSchema(const char* xsd)
    {
        beresta_schema schema = nullptr;

        beresta_schema_load(xsd, static_cast<int64_t>(std::strlen(xsd)), &schema);
        return schema;
    }

    // Строки удобно сравнивать целиком: тексты, строки и столбцы в одном значении
    // Strings are convenient to compare entirely: texts, lines and columns in one value
    std::vector<std::string> takeCapiDiagnostics(beresta_diagnostics diagnostics)
    {
        std::vector<std::string> result;
        int64_t count = beresta_diagnostics_count(diagnostics);

        for (int64_t index = 0; index < count; ++index)
            result.push_back(std::to_string(beresta_diagnostics_line(diagnostics, index)) + ":"
                + std::to_string(beresta_diagnostics_column(diagnostics, index)) + ": "
                + takeCapiString(beresta_diagnostics_message(diagnostics, index)));
        beresta_diagnostics_free(diagnostics);
        return result;
    }

    const char* const ORDER_XSD =
        "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
        "<xs:element name='order'><xs:complexType><xs:sequence>"
        "<xs:element name='item' maxOccurs='unbounded'><xs:complexType><xs:simpleContent>"
        "<xs:extension base='xs:string'>"
        "<xs:attribute name='qty' type='xs:positiveInteger' use='required'/>"
        "</xs:extension></xs:simpleContent></xs:complexType></xs:element>"
        "</xs:sequence></xs:complexType></xs:element></xs:schema>";

    const char* const ORDER_XML =
        "<order>\n<item qty='2'>чай</item>\n<item qty='0'>кофе</item>\n<item>мёд</item>\n</order>";
}
