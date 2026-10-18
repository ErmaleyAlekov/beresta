#include "bench/engines/xerces_engine.hpp"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XercesVersion.hpp>

namespace bench
{
    // Xerces требует глобальной инициализации; разбор — без проверки, с пространствами имён
    // Xerces needs a global initialization; parsing — without validation, with namespaces
    XercesEngine::XercesEngine()
        : _document(nullptr)
    {
        xercesc::XMLPlatformUtils::Initialize();
        _parser = std::make_unique<xercesc::XercesDOMParser>();
        _parser->setDoNamespaces(true);
        _parser->setValidationScheme(xercesc::XercesDOMParser::Val_Never);
    }

    // Парсеры и дерево освобождаются ДО Terminate / The parsers and the tree are freed BEFORE Terminate
    XercesEngine::~XercesEngine()
    {
        release();
        _parser.reset();
        _validating.reset();
        xercesc::XMLPlatformUtils::Terminate();
    }

    // Название и версия / The name and the version
    std::string XercesEngine::name() const
    {
        return std::string("Xerces-C++ ") + XERCES_FULLVERSIONDOT;
    }

    // Разбор из памяти; дерево забирается у парсера (adoptDocument) / Parsing from memory; the tree is taken from the parser (adoptDocument)
    void XercesEngine::parse(const std::string& xml)
    {
        xercesc::MemBufInputSource source(reinterpret_cast<const XMLByte*>(xml.data()), xml.size(), "bench");

        release();
        _parser->parse(source);
        if (_parser->getErrorCount() != 0)
            throw BenchError("Xerces: документ отвергнут / the document is rejected");
        _document = _parser->adoptDocument();
    }

    // Освобождение дерева / Freeing the tree
    void XercesEngine::release()
    {
        if (_document != nullptr)
            _document->release();
        _document = nullptr;
    }
}
