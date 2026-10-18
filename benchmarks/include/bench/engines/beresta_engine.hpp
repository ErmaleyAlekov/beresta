#ifndef BERESTA_BENCH_BERESTA_ENGINE_HPP
#define BERESTA_BENCH_BERESTA_ENGINE_HPP

#include "bench/engine.hpp"
#include "beresta.hpp"

#include <memory>

namespace bench
{
    /*
    ** «Берёста» через свой C++ API (как и остальные библиотеки — через их
    ** родной API, без промежуточных слоёв). Разбор с настройками по
    ** умолчанию: пространства имён включены, лимиты защиты включены всегда.
    **
    ** Beresta through its C++ API (like the other libraries — through their
    ** native API, without intermediate layers). Parsing with the default
    ** settings: namespaces are on, the protective limits are always on.
    */
    class BerestaEngine : public XmlEngine
    {
    public:
        std::string name() const override;
        void parse(const std::string& xml) override;
        std::size_t traverse() const override;
        void release() override;
        std::optional<std::size_t> serialize() const override;
        std::optional<std::size_t> xpathCount(const std::string& query) const override;
        std::optional<std::size_t> parseValidated(const std::string& xml, const std::string& xsd) override;
        std::optional<std::size_t> streamValidated(const std::string& xml, const std::string& xsd) override;

    private:
        std::unique_ptr<beresta::Document> _document;
        std::shared_ptr<const beresta::xsd::Schema> _schema;
    };
}

#endif
