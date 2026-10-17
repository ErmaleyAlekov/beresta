#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include "exception/parse_exception.hpp"

#include <string>
#include <vector>

namespace beresta
{
    // Серьёзность проблемы
    enum class ErrorSeverity
    {
        WARNING,            // Предупреждение: документ корректен, но подозрителен
        RECOVERABLE_ERROR,  // Ошибка валидности (DTD / XSD): разбор можно продолжить
        FATAL_ERROR         // Нарушение синтаксиса XML: разбор невозможен
        // Имя ERROR не используется намеренно: <windows.h> определяет макрос ERROR
    };

    /*
    ** ErrorHandler — получатель проблем разбора (аналог ErrorHandler в Xerces).
    **
    ** По стандарту XML нарушение синтаксиса (well-formedness) всегда
    ** фатально: после fatalError() парсер в любом случае бросает исключение.
    ** Ошибки валидности (DTD, XSD) восстановимы: если error() не бросает,
    ** разбор продолжается — так за один проход собираются ВСЕ ошибки.
    **
    ** Реализация по умолчанию: предупреждения игнорируются, ошибки бросаются.
    */
    class ErrorHandler
    {
    public:
        virtual ~ErrorHandler();

        virtual void warning(const ParseException& problem);
        virtual void error(const ParseException& problem);
        virtual void fatalError(const ParseException& problem);
    };

    // Одна собранная проблема
    struct Diagnostic
    {
        ErrorSeverity severity;
        std::string message;
        ParsePosition position;
    };

    /*
    ** CollectingErrorHandler — готовый обработчик, который ничего не бросает
    ** (кроме фатальных ошибок — их бросает сам парсер) и копит все проблемы.
    ** Удобен для валидации: разобрали, затем вывели полный список ошибок.
    */
    class CollectingErrorHandler : public ErrorHandler
    {
    public:
        void warning(const ParseException& problem) override;
        void error(const ParseException& problem) override;
        void fatalError(const ParseException& problem) override;

        // Все собранные проблемы в порядке появления
        const std::vector<Diagnostic>& getDiagnostics() const noexcept;

        // Есть ли хоть одна ошибка (RECOVERABLE_ERROR или FATAL_ERROR)
        bool hasErrors() const noexcept;

        // Очищает список (для повторного использования обработчика)
        void clear() noexcept;

    private:
        // Сохраняет проблему с указанной серьёзностью
        void record(ErrorSeverity severity, const ParseException& problem);

        std::vector<Diagnostic> _diagnostics;
    };
}

#endif
