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
    **
    ** An own mini testing framework (without external dependencies).
    ** A failed check throws TestFailure; the runner catches it,
    ** prints the place and the reason and moves on to the next test.
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
    // A test is an ordinary function without arguments (without lambdas and hidden registration)
    using TestFunction = void (*)();

    // Описание одного теста: имя для отчёта и функция
    // The description of one test: the name for the report and the function
    struct TestCase
    {
        std::string name;
        TestFunction function;
    };

    /*
    ** TestRunner — реестр тестов и исполнитель.
    ** Регистрация явная: каждый набор тестов сам добавляет свои функции.
    **
    ** TestRunner — the test registry and executor.
    ** Registration is explicit: each test suite adds its functions itself.
    */
    class TestRunner
    {
    public:
        // Регистрирует тест
        // Registers a test
        void add(std::string name, TestFunction function);

        // Запускает все тесты, печатает отчёт; возвращает число упавших
        // Runs all tests, prints the report; returns the number of failed ones
        int runAll() const;

    private:
        // Запускает один тест; истина — тест прошёл
        // Runs one test; true — the test passed
        bool runOne(const TestCase& testCase) const;

        std::vector<TestCase> _cases;
    };

    // Проверка истинности условия
    // Checks that a condition is true
    void checkTrue(bool condition, const char* expression, const char* file, int line);

    // Проверка равенства строк с выводом обоих значений при расхождении
    // Checks that strings are equal, printing both values on a mismatch
    void checkEqual(const std::string& actual, const std::string& expected,
        const char* expression, const char* file, int line);

    // Переключает консоль Windows в UTF-8 (на Unix ничего не делает)
    // Switches the Windows console to UTF-8 (does nothing on Unix)
    void enableUtf8Console() noexcept;
}

// Условие должно быть истинным
// The condition must be true
#define TEST_ASSERT(condition) \
    ::test::checkTrue((condition), #condition, __FILE__, __LINE__)

// Строковое значение должно совпасть с ожидаемым
// The string value must match the expected one
#define TEST_ASSERT_EQ(actual, expected) \
    ::test::checkEqual((actual), (expected), #actual, __FILE__, __LINE__)

// Инструкция обязана бросить исключение указанного типа
// The statement must throw an exception of the given type
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
