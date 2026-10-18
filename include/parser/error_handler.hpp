#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include "exception/parse_exception.hpp"

#include <string>
#include <vector>

namespace beresta
{
    // Серьёзность проблемы
    // The severity of a problem
    enum class ErrorSeverity
    {
        WARNING,            // Предупреждение: документ корректен, но подозрителен
                            // A warning: the document is valid but suspicious
        RECOVERABLE_ERROR,  // Ошибка валидности (DTD / XSD): разбор можно продолжить
                            // A validity error (DTD / XSD): parsing can continue
        FATAL_ERROR         // Нарушение синтаксиса XML: разбор невозможен
                            // An XML syntax violation: parsing is impossible
        // Имя ERROR не используется намеренно: <windows.h> определяет макрос ERROR
        // The name ERROR is deliberately not used: <windows.h> defines an ERROR macro
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
    **
    ** ErrorHandler — the receiver of parsing problems (the analog of ErrorHandler in Xerces).
    ** By the XML standard a syntax (well-formedness) violation is always
    ** fatal: after fatalError() the parser throws an exception in any case.
    ** Validity errors (DTD, XSD) are recoverable: if error() does not throw,
    ** parsing continues — so ALL errors are collected in one pass.
    ** The default implementation: warnings are ignored, errors are thrown.
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
    // One collected problem
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
    **
    ** CollectingErrorHandler — a ready handler that throws nothing
    ** (except fatal errors — the parser itself throws them) and accumulates all problems.
    ** Convenient for validation: parse, then print the full list of errors.
    */
    class CollectingErrorHandler : public ErrorHandler
    {
    public:
        void warning(const ParseException& problem) override;
        void error(const ParseException& problem) override;
        void fatalError(const ParseException& problem) override;

        // Все собранные проблемы в порядке появления
        // All collected problems in the order they appeared
        const std::vector<Diagnostic>& getDiagnostics() const noexcept;

        // Есть ли хоть одна ошибка (RECOVERABLE_ERROR или FATAL_ERROR)
        // Whether there is at least one error (RECOVERABLE_ERROR or FATAL_ERROR)
        bool hasErrors() const noexcept;

        // Очищает список (для повторного использования обработчика)
        // Clears the list (to reuse the handler)
        void clear() noexcept;

    private:
        // Сохраняет проблему с указанной серьёзностью
        // Stores a problem with the given severity
        void record(ErrorSeverity severity, const ParseException& problem);

        std::vector<Diagnostic> _diagnostics;
    };
}

#endif
