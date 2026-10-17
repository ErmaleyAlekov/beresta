#include "parser/error_handler.hpp"

namespace beresta
{
    // Предупреждение сохраняется
    void CollectingErrorHandler::warning(const ParseException& problem)
    {
        record(ErrorSeverity::WARNING, problem);
    }

    // Ошибка валидности сохраняется, разбор продолжается
    void CollectingErrorHandler::error(const ParseException& problem)
    {
        record(ErrorSeverity::RECOVERABLE_ERROR, problem);
    }

    // Фатальная ошибка сохраняется; исключение затем бросит сам парсер
    void CollectingErrorHandler::fatalError(const ParseException& problem)
    {
        record(ErrorSeverity::FATAL_ERROR, problem);
    }

    // Копируем текст без префикса кода и позицию
    void CollectingErrorHandler::record(ErrorSeverity severity, const ParseException& problem)
    {
        _diagnostics.push_back(Diagnostic{severity, problem.getMessage(), problem.getPosition()});
    }
}
