#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP

#include <exception>
#include <string>
#include <vector>

namespace test
{
    /*
    ** Собственный мини-фреймворк тестирования (без внешних зависимостей).
    ** Проверка, которая не прошла, бросает TestFailure; раннер ловит его,
    ** печатает место и причину и переходит к следующему тесту.
    */
    class TestFailure : public std::exception
    {
    public:
        explicit TestFailure(std::string message);
        const char* what() const noexcept override;

    private:
        std::string _message;
    };

    // Тест — обычная функция без аргументов (без лямбд и скрытой регистрации)
    using TestFunction = void (*)();

    // Описание одного теста: имя для отчёта и функция
    struct TestCase
    {
        std::string name;
        TestFunction function;
    };

    /*
    ** TestRunner — реестр тестов и исполнитель.
    ** Регистрация явная: каждый набор тестов сам добавляет свои функции.
    */
    class TestRunner
    {
    public:
        // Регистрирует тест
        void add(std::string name, TestFunction function);

        // Запускает все тесты, печатает отчёт; возвращает число упавших
        int runAll() const;

    private:
        // Запускает один тест; истина — тест прошёл
        bool runOne(const TestCase& testCase) const;

        std::vector<TestCase> _cases;
    };

    // Проверка истинности условия
    void checkTrue(bool condition, const char* expression, const char* file, int line);

    // Проверка равенства строк с выводом обоих значений при расхождении
    void checkEqual(const std::string& actual, const std::string& expected,
        const char* expression, const char* file, int line);

    // Переключает консоль Windows в UTF-8 (на Unix ничего не делает)
    void enableUtf8Console() noexcept;
}

// Условие должно быть истинным
#define TEST_ASSERT(condition) \
    ::test::checkTrue((condition), #condition, __FILE__, __LINE__)

// Строковое значение должно совпасть с ожидаемым
#define TEST_ASSERT_EQ(actual, expected) \
    ::test::checkEqual((actual), (expected), #actual, __FILE__, __LINE__)

// Инструкция обязана бросить исключение указанного типа
#define TEST_ASSERT_THROWS(statement, ExceptionType)                          \
    do                                                                        \
    {                                                                         \
        bool thrown_ = false;                                                 \
        try                                                                   \
        {                                                                     \
            statement;                                                        \
        }                                                                     \
        catch (const ExceptionType&)                                          \
        {                                                                     \
            thrown_ = true;                                                   \
        }                                                                     \
        ::test::checkTrue(thrown_, "ожидалось исключение " #ExceptionType     \
            " из: " #statement, __FILE__, __LINE__);                          \
    } while (0)

#endif
