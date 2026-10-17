#include "parser/error_handler.hpp"

namespace beresta
{
    // Собранные проблемы
    const std::vector<Diagnostic>& CollectingErrorHandler::getDiagnostics() const noexcept
    {
        return _diagnostics;
    }

    // Предупреждения ошибками не считаются
    bool CollectingErrorHandler::hasErrors() const noexcept
    {
        for (const Diagnostic& diagnostic : _diagnostics)
        {
            if (diagnostic.severity != ErrorSeverity::WARNING)
                return true;
        }
        return false;
    }

    // Очистка списка
    void CollectingErrorHandler::clear() noexcept
    {
        _diagnostics.clear();
    }
}
