#include "bench/engine.hpp"

#include <utility>

namespace bench
{
    // Сообщение хранится готовым: what() ничего не выделяет / The message is stored ready: what() allocates nothing
    BenchError::BenchError(std::string message)
        : _message(std::move(message))
    {
    }

    // Текст ошибки / The error text
    const char* BenchError::what() const noexcept
    {
        return _message.c_str();
    }

    // Виртуальный деструктор интерфейса / The virtual destructor of the interface
    XmlEngine::~XmlEngine() = default;

    // По умолчанию сериализация не поддерживается / By default serialization is not supported
    std::optional<std::size_t> XmlEngine::serialize() const
    {
        return std::nullopt;
    }

    // По умолчанию XPath не поддерживается / By default XPath is not supported
    std::optional<std::size_t> XmlEngine::xpathCount(const std::string& query) const
    {
        (void)query;
        return std::nullopt;
    }
}
