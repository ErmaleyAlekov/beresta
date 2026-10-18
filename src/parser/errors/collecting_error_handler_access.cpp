#include "parser/error_handler.hpp"

namespace beresta
{
    // Собранные проблемы
    // The collected problems
    const std::vector<Diagnostic>& CollectingErrorHandler::getDiagnostics() const noexcept
    {
        return _diagnostics;
    }

    // Предупреждения ошибками не считаются
    // Warnings are not counted as errors
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
    // Clears the list
    void CollectingErrorHandler::clear() noexcept
    {
        _diagnostics.clear();
    }
}
