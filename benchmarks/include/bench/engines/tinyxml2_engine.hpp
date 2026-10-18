#ifndef BERESTA_BENCH_TINYXML2_ENGINE_HPP
#define BERESTA_BENCH_TINYXML2_ENGINE_HPP

#include "bench/engine.hpp"

#include <tinyxml2.h>

namespace bench
{
    /*
    ** tinyxml2 — маленький DOM-парсер без проверки по DTD/XSD, без XPath и
    ** без пространств имён. Популярен во встраиваемых системах и играх.
    **
    ** tinyxml2 — a small DOM parser without DTD/XSD validation, without XPath and
    ** without namespaces. Popular in embedded systems and games.
    */
    class Tinyxml2Engine : public XmlEngine
    {
    public:
        std::string name() const override;
        void parse(const std::string& xml) override;
        std::size_t traverse() const override;
        void release() override;
        std::optional<std::size_t> serialize() const override;

    private:
        tinyxml2::XMLDocument _document;
    };
}

#endif
