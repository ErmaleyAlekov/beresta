#ifndef CONFORMANCE_ERROR_HPP
#define CONFORMANCE_ERROR_HPP

#include <exception>
#include <string>

namespace conformance
{
    /*
    ** Собственное исключение прогонщика: сбой самой инфраструктуры
    ** (нет каталога, не читается файл ожидаемого вывода, неверные
    ** аргументы командной строки). Отделено от исключений библиотеки,
    ** чтобы ошибку окружения нельзя было спутать с вердиктом по случаю.
    **
    ** The runner's own exception: a failure of the infrastructure itself
    ** (no catalog, the expected output file cannot be read, invalid
    ** command line arguments). It is separate from the library exceptions
    ** so that an environment error cannot be confused with a case verdict.
    */
    class ConformanceError : public std::exception
    {
    public:
        explicit ConformanceError(std::string message);

        // Текст ошибки для вывода пользователю
        // The error text for showing to the user
        const char* what() const noexcept override;

    private:
        std::string _message;
    };
}

#endif
