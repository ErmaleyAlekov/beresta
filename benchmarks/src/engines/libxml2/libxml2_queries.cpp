#include "bench/engines/libxml2_engine.hpp"

#include <libxml/xpath.h>

namespace bench
{
    // Вывод в память; размер включает XML-декларацию, как и у остальных / Output to memory; the size includes the XML declaration, as for the others
    std::optional<std::size_t> Libxml2Engine::serialize() const
    {
        xmlChar* buffer = nullptr;
        int size = 0;

        if (!_document)
            return 0;
        xmlDocDumpMemory(_document, &buffer, &size);
        if (!buffer)
            throw BenchError("libxml2: ошибка записи / serialization failed");
        xmlFree(buffer);
        return static_cast<std::size_t>(size);
    }

    // Компиляция и вычисление выражения XPath 1.0 / Compiling and evaluating an XPath 1.0 expression
    std::optional<std::size_t> Libxml2Engine::xpathCount(const std::string& query) const
    {
        if (!_document)
            return 0;
        xmlXPathContextPtr context = xmlXPathNewContext(_document);
        if (!context)
            throw BenchError("libxml2: нет контекста / missing XPath context");
        xmlXPathObjectPtr result = xmlXPathEvalExpression(reinterpret_cast<const xmlChar*>(query.c_str()), context);
        std::size_t count = 0;

        if (result != nullptr && result->nodesetval != nullptr)
            count = static_cast<std::size_t>(result->nodesetval->nodeNr);
        bool failed = result == nullptr;
        xmlXPathFreeObject(result);
        xmlXPathFreeContext(context);
        if (failed)
            throw BenchError("libxml2: ошибка XPath / XPath evaluation failed");
        return count;
    }

    /*
    ** Схема компилируется один раз; документ разбирается в дерево и
    ** проверяется xmlSchemaValidateDoc вторым проходом — обе стадии входят
    ** в замер, как у «Берёсты» с ParserOptions::schema.
    **
    ** The schema is compiled once; the document is parsed into a tree and
    ** checked by xmlSchemaValidateDoc in a second pass — both stages go into
    ** the measurement, as for Beresta with ParserOptions::schema.
    */
    std::optional<std::size_t> Libxml2Engine::parseValidated(const std::string& xml, const std::string& xsd)
    {
        std::size_t errors = 0;

        xmlSchemaPtr schema = schemaFor(xsd);
        xmlDocPtr document = xmlReadMemory(xml.data(), bufferLength(xml.size()), nullptr, nullptr, XML_PARSE_NONET);
        if (!document)
            throw BenchError("libxml2: документ отвергнут / document rejected");
        xmlSchemaValidCtxtPtr validator = xmlSchemaNewValidCtxt(schema);
        if (!validator)
        {
            xmlFreeDoc(document);
            throw BenchError("libxml2: нет контекста / missing validation context");
        }
        xmlSchemaSetValidStructuredErrors(validator, countLibxml2Error, &errors);
        int status = xmlSchemaValidateDoc(validator, document);
        xmlSchemaFreeValidCtxt(validator);
        xmlFreeDoc(document);
        if (status < 0)
            throw BenchError("libxml2: сбой проверки / validation failed");
        return errors;
    }
}
