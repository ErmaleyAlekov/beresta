#include "bench/engines/libxml2_engine.hpp"

#include <libxml/xmlversion.h>
#include <limits>

namespace bench
{
    // Глобальная инициализация libxml2 (повторный вызов безопасен) / The global libxml2 initialization (a repeated call is safe)
    Libxml2Engine::Libxml2Engine()
        : _document(nullptr), _schema(nullptr)
    {
        xmlInitParser();
    }

    // Дерево и скомпилированная схема освобождаются / The tree and the compiled schema are freed
    Libxml2Engine::~Libxml2Engine()
    {
        release();
        if (_schema != nullptr)
            xmlSchemaFree(_schema);
    }

    // Название и версия / The name and the version
    std::string Libxml2Engine::name() const
    {
        return std::string("libxml2 ") + LIBXML_DOTTED_VERSION;
    }

    // Длина буфера C API / The C API buffer length
    int Libxml2Engine::bufferLength(std::size_t size)
    {
        if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw BenchError("libxml2: слишком большой буфер / buffer too large");
        return static_cast<int>(size);
    }

    // Разбор из памяти; NULL — документ отвергнут / Parsing from memory; NULL means the document is rejected
    void Libxml2Engine::parse(const std::string& xml)
    {
        auto document = xmlReadMemory(xml.data(), bufferLength(xml.size()), nullptr, nullptr, XML_PARSE_NONET);
        if (!document)
            throw BenchError("libxml2: документ отвергнут / the document is rejected");
        release();
        _document = document;
    }

    // Освобождение дерева / Freeing the tree
    void Libxml2Engine::release()
    {
        if (_document != nullptr)
            xmlFreeDoc(_document);
        _document = nullptr;
    }
}
