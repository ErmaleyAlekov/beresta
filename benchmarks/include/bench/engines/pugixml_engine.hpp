#ifndef BERESTA_BENCH_PUGIXML_ENGINE_HPP
#define BERESTA_BENCH_PUGIXML_ENGINE_HPP

#include "bench/engine.hpp"

#include <pugixml.hpp>

namespace bench
{
    /*
    ** pugixml — один из самых быстрых DOM-парсеров на C++ (с XPath 1.0).
    ** Не проверяет DTD и XSD, не знает пространств имён и кодировок кроме
    ** UTF-8/16/32 и Latin-1: часть работы, которую делает «Берёста», он
    ** не выполняет вовсе. Разбор parse_default.
    **
    ** pugixml is one of the fastest C++ DOM parsers (with XPath 1.0).
    ** It does not check DTD and XSD, knows no namespaces and no encodings except
    ** UTF-8/16/32 and Latin-1: part of the work Beresta does it does not do at
    ** all. Parsing with parse_default.
    */
    class PugixmlEngine : public XmlEngine
    {
    public:
        std::string name() const override;
        void parse(const std::string& xml) override;
        std::size_t traverse() const override;
        void release() override;
        std::optional<std::size_t> serialize() const override;
        std::optional<std::size_t> xpathCount(const std::string& query) const override;

    private:
        pugi::xml_document _document;
    };
}

#endif
