#ifndef BERESTA_BENCH_LIBXML2_ENGINE_HPP
#define BERESTA_BENCH_LIBXML2_ENGINE_HPP

#include "bench/engine.hpp"

#include <libxml/parser.h>
#include <libxml/xmlschemas.h>

namespace bench
{
    /*
    ** libxml2 (GNOME) — самый распространённый XML-парсер на C: GNOME,
    ** Chromium, PHP, Python lxml, Node libxmljs. Разбор xmlReadMemory с
    ** XML_PARSE_NONET (без сети — как «Берёста» по умолчанию).
    **
    ** libxml2 (GNOME) — the most widespread C XML parser: GNOME, Chromium,
    ** PHP, Python lxml, Node libxmljs. Parsing with xmlReadMemory and
    ** XML_PARSE_NONET (no network — like Beresta by default).
    */
    class Libxml2Engine : public XmlEngine
    {
    public:
        Libxml2Engine();
        ~Libxml2Engine() override;
        std::string name() const override;
        void parse(const std::string& xml) override;
        std::size_t traverse() const override;
        void release() override;
        std::optional<std::size_t> serialize() const override;
        std::optional<std::size_t> xpathCount(const std::string& query) const override;
        std::optional<std::size_t> parseValidated(const std::string& xml, const std::string& xsd) override;

    private:
        xmlDocPtr _document;
        xmlSchemaPtr _schema;
    };
}

#endif
