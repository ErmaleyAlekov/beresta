#include "parser/error_handler.hpp"

namespace beresta
{
    // Виртуальный деструктор для безопасного удаления наследников
    ErrorHandler::~ErrorHandler()
    {
    }

    // Предупреждения по умолчанию игнорируются: документ всё равно корректен
    void ErrorHandler::warning(const ParseException& problem)
    {
        (void)problem;
    }

    // Ошибка валидности по умолчанию прерывает разбор (fail-fast)
    void ErrorHandler::error(const ParseException& problem)
    {
        throw problem;
    }

    // Фатальная ошибка по умолчанию пробрасывается как есть
    void ErrorHandler::fatalError(const ParseException& problem)
    {
        throw problem;
    }
}
