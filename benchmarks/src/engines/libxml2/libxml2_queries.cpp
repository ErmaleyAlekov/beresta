#include "bench/engines/libxml2_engine.hpp"

#include <libxml/xpath.h>

namespace bench
{
    // Вывод в память; размер включает XML-декларацию, как и у остальных / Output to memory; the size includes the XML declaration, as for the others
    std::optional<std::size_t> Libxml2Engine::serialize() const
    {
        xmlChar* buffer = nullptr;
        int size = 0;

        xmlDocDumpMemory(_document, &buffer, &size);
        xmlFree(buffer);
        return static_cast<std::size_t>(size);
    }

    // Компиляция и вычисление выражения XPath 1.0 / Compiling and evaluating an XPath 1.0 expression
    std::optional<std::size_t> Libxml2Engine::xpathCount(const std::string& query) const
    {
        xmlXPathContextPtr context = xmlXPathNewContext(_document);
        xmlXPathObjectPtr result = xmlXPathEvalExpression(reinterpret_cast<const xmlChar*>(query.c_str()), context);
        std::size_t count = 0;

        if (result != nullptr && result->nodesetval != nullptr)
            count = static_cast<std::size_t>(result->nodesetval->nodeNr);
        xmlXPathFreeObject(result);
        xmlXPathFreeContext(context);
        return count;
    }

    // Счётчик ошибок валидации (обработчик libxml2) / The validation error counter (a libxml2 handler)
    static void countError(void* data, const xmlError* error)
    {
        (void)error;
        ++*static_cast<std::size_t*>(data);
    }

    /*
    ** Схема компилируется один раз; документ разбирается и проверяется
    ** xmlSchemaValidateDoc — обе стадии входят в замер, как у «Берёсты».
    **
    ** The schema is compiled once; the document is parsed and checked with
    ** xmlSchemaValidateDoc — both stages go into the measurement, as for Beresta.
    */
    std::optional<std::size_t> Libxml2Engine::parseValidated(const std::string& xml, const std::string& xsd)
    {
        std::size_t errors = 0;

        if (_schema == nullptr)
        {
            xmlSchemaParserCtxtPtr parser = xmlSchemaNewMemParserCtxt(xsd.data(), static_cast<int>(xsd.size()));
            _schema = xmlSchemaParse(parser);
            xmlSchemaFreeParserCtxt(parser);
        }
        xmlDocPtr document = xmlReadMemory(xml.data(), static_cast<int>(xml.size()), nullptr, nullptr, XML_PARSE_NONET);
        xmlSchemaValidCtxtPtr validator = xmlSchemaNewValidCtxt(_schema);
        xmlSchemaSetValidStructuredErrors(validator, countError, &errors);
        xmlSchemaValidateDoc(validator, document);
        xmlSchemaFreeValidCtxt(validator);
        xmlFreeDoc(document);
        return errors;
    }
}
