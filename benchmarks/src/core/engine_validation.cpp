#include "bench/engine.hpp"

namespace bench
{
    // По умолчанию проверка по XML Schema не поддерживается / By default XML Schema validation is not supported
    std::optional<std::size_t> XmlEngine::parseValidated(const std::string& xml, const std::string& xsd)
    {
        (void)xml;
        (void)xsd;
        return std::nullopt;
    }

    // По умолчанию потоковой проверки нет / By default there is no streaming validation
    std::optional<std::size_t> XmlEngine::streamValidated(const std::string& xml, const std::string& xsd)
    {
        (void)xml;
        (void)xsd;
        return std::nullopt;
    }
}
