#include "failing/list_error.hpp"

namespace failing
{
    ListError::ListError(const std::string& message)
        : xsts::XstsError(message)
    {
    }
}
