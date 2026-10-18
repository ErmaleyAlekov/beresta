#include "capi/internal/capi_support.hpp"

namespace beresta::capi
{
    InvalidArgumentException::InvalidArgumentException(std::string message)
        : _message(std::move(message))
    {
    }

    // Сообщение хранится готовым: what() ничего не выделяет
    // The message is stored ready: what() allocates nothing
    const char* InvalidArgumentException::what() const noexcept
    {
        return _message.c_str();
    }
}
