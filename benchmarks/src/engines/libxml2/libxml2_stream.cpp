#include "bench/engines/libxml2_engine.hpp"

namespace bench
{
    // Каждая ошибка валидации увеличивает счётчик / Every validation error increases the counter
    void countLibxml2Error(void* data, const xmlError* error)
    {
        (void)error;
        ++*static_cast<std::size_t*>(data);
    }

    // Схема компилируется при первом обращении и дальше переиспользуется / The schema is compiled on the first call and reused afterwards
    xmlSchemaPtr Libxml2Engine::schemaFor(const std::string& xsd)
    {
        if (_schema == nullptr)
        {
            xmlSchemaParserCtxtPtr parser = xmlSchemaNewMemParserCtxt(xsd.data(), static_cast<int>(xsd.size()));
            _schema = xmlSchemaParse(parser);
            xmlSchemaFreeParserCtxt(parser);
        }
        return _schema;
    }

    /*
    ** Потоковая проверка libxml2: xmlSchemaValidateStream разбирает вход
    ** SAX-парсером и проверяет события сразу, без дерева. Буфер ввода
    ** переходит во владение контекста разбора и освобождается вместе с ним.
    **
    ** Streaming libxml2 validation: xmlSchemaValidateStream parses the input with
    ** the SAX parser and checks the events at once, without a tree. The input buffer
    ** passes into the ownership of the parser context and is freed together with it.
    */
    std::optional<std::size_t> Libxml2Engine::streamValidated(const std::string& xml, const std::string& xsd)
    {
        std::size_t errors = 0;
        xmlSchemaValidCtxtPtr validator = xmlSchemaNewValidCtxt(schemaFor(xsd));
        xmlParserInputBufferPtr input = xmlParserInputBufferCreateMem(xml.data(), static_cast<int>(xml.size()),
            XML_CHAR_ENCODING_NONE);

        xmlSchemaSetValidStructuredErrors(validator, countLibxml2Error, &errors);
        xmlSchemaValidateStream(validator, input, XML_CHAR_ENCODING_NONE, nullptr, nullptr);
        xmlSchemaFreeValidCtxt(validator);
        return errors;
    }
}
