#include "framework/test_framework.hpp"

#include <utility>

#ifdef _WIN32
# include <windows.h>
#endif

namespace test
{
    // Исключение проваленной проверки хранит готовое сообщение
    TestFailure::TestFailure(std::string message)
        : _message(std::move(message))
    {
    }

    // Сообщение о провале
    const char* TestFailure::what() const noexcept
    {
        return _message.c_str();
    }

    // Ложное условие превращается в TestFailure с указанием файла и строки
    void checkTrue(bool condition, const char* expression, const char* file, int line)
    {
        if (condition)
            return;
        throw TestFailure(std::string(file) + ":" + std::to_string(line)
            + ": проверка не прошла: " + expression);
    }

    // При расхождении показываем и фактическое, и ожидаемое значение
    void checkEqual(const std::string& actual, const std::string& expected,
        const char* expression, const char* file, int line)
    {
        if (actual == expected)
            return;
        throw TestFailure(std::string(file) + ":" + std::to_string(line)
            + ": " + expression + "\n       получено:  [" + actual
            + "]\n       ожидалось: [" + expected + "]");
    }

    // Консоль Windows по умолчанию не в UTF-8 — переключаем для кириллицы
    void enableUtf8Console() noexcept
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
#endif
    }
}
