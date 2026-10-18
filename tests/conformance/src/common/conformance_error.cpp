#include "conformance/conformance_error.hpp"

#include <utility>

namespace conformance
{
    ConformanceError::ConformanceError(std::string message)
        : _message(std::move(message))
    {
    }

    // Сообщение хранится в объекте, поэтому указатель живёт столько же, сколько исключение
    // The message is stored in the object, so the pointer lives as long as the exception
    const char* ConformanceError::what() const noexcept
    {
        return _message.c_str();
    }
}
