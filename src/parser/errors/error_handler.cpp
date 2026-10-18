#include "parser/error_handler.hpp"

namespace beresta
{
    // Виртуальный деструктор для безопасного удаления наследников
    // A virtual destructor for safe deletion of subclasses
    ErrorHandler::~ErrorHandler()
    {
    }

    // Предупреждения по умолчанию игнорируются: документ всё равно корректен
    // Warnings are ignored by default: the document is valid anyway
    void ErrorHandler::warning(const ParseException& problem)
    {
        (void)problem;
    }

    // Ошибка валидности по умолчанию прерывает разбор (fail-fast)
    // A validity error stops parsing by default (fail-fast)
    void ErrorHandler::error(const ParseException& problem)
    {
        throw problem;
    }

    // Фатальная ошибка по умолчанию пробрасывается как есть
    // A fatal error is rethrown as is by default
    void ErrorHandler::fatalError(const ParseException& problem)
    {
        throw problem;
    }
}
