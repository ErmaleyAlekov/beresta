#include "bench/engines/libxml2_engine.hpp"

#include <libxml/xmlversion.h>

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

    // Разбор из памяти; NULL — документ отвергнут / Parsing from memory; NULL means the document is rejected
    void Libxml2Engine::parse(const std::string& xml)
    {
        release();
        _document = xmlReadMemory(xml.data(), static_cast<int>(xml.size()), nullptr, nullptr, XML_PARSE_NONET);
        if (_document == nullptr)
            throw BenchError("libxml2: документ отвергнут / the document is rejected");
    }

    // Освобождение дерева / Freeing the tree
    void Libxml2Engine::release()
    {
        if (_document != nullptr)
            xmlFreeDoc(_document);
        _document = nullptr;
    }
}
