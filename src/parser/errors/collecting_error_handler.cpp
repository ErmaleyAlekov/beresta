#include "parser/error_handler.hpp"

namespace beresta
{
    // Предупреждение сохраняется
    // A warning is stored
    void CollectingErrorHandler::warning(const ParseException& problem)
    {
        record(ErrorSeverity::WARNING, problem);
    }

    // Ошибка валидности сохраняется, разбор продолжается
    // A validity error is stored, parsing continues
    void CollectingErrorHandler::error(const ParseException& problem)
    {
        record(ErrorSeverity::RECOVERABLE_ERROR, problem);
    }

    // Фатальная ошибка сохраняется; исключение затем бросит сам парсер
    // A fatal error is stored; then the parser itself throws the exception
    void CollectingErrorHandler::fatalError(const ParseException& problem)
    {
        record(ErrorSeverity::FATAL_ERROR, problem);
    }

    // Копируем текст без префикса кода и позицию
    // Copies the text without the code prefix and the position
    void CollectingErrorHandler::record(ErrorSeverity severity, const ParseException& problem)
    {
        _diagnostics.push_back(Diagnostic{severity, problem.getMessage(), problem.getPosition()});
    }
}
