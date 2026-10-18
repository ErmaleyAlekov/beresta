#ifndef BERESTA_BENCH_XERCES_ENGINE_HPP
#define BERESTA_BENCH_XERCES_ENGINE_HPP

#include "bench/engine.hpp"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <memory>

namespace bench
{
    /*
    ** Xerces-C++ (Apache) — библиотека, которую заменяет «Берёста»: полный
    ** DOM, DTD и XML Schema. XPath общего вида в Xerces нет (DOMXPath умеет
    ** лишь подмножество для ограничений идентичности XSD), поэтому столбец
    ** XPath у него пуст. Строки внутри — UTF-16 (XMLCh).
    **
    ** Xerces-C++ (Apache) — the library Beresta replaces: full DOM, DTD and
    ** XML Schema. Xerces has no general XPath (DOMXPath supports only a subset
    ** for XSD identity constraints), so its XPath column is empty. The strings
    ** inside are UTF-16 (XMLCh).
    */
    class XercesEngine : public XmlEngine
    {
    public:
        XercesEngine();
        ~XercesEngine() override;
        std::string name() const override;
        void parse(const std::string& xml) override;
        std::size_t traverse() const override;
        void release() override;
        std::optional<std::size_t> serialize() const override;
        std::optional<std::size_t> parseValidated(const std::string& xml, const std::string& xsd) override;

    private:
        std::unique_ptr<xercesc::XercesDOMParser> _parser;
        std::unique_ptr<xercesc::XercesDOMParser> _validating;
        xercesc::DOMDocument* _document;
    };
}

#endif
