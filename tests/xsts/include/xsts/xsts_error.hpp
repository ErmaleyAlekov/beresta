#ifndef XSTS_ERROR_HPP
#define XSTS_ERROR_HPP

#include <exception>
#include <string>

namespace xsts
{
    /*
    ** Собственное исключение прогонщика: сбой самой инфраструктуры
    ** (нет набора, не читается файл метаданных, неверные аргументы
    ** командной строки). Отделено от исключений библиотеки, чтобы ошибку
    ** окружения нельзя было спутать с вердиктом по случаю.
    **
    ** The runner's own exception: a failure of the infrastructure itself
    ** (no suite, a metadata file cannot be read, invalid command line
    ** arguments). It is separate from the library exceptions so that an
    ** environment error cannot be confused with a case verdict.
    */
    class XstsError : public std::exception
    {
    public:
        explicit XstsError(std::string message);

        // Текст ошибки для вывода пользователю
        // The error text for showing to the user
        const char* what() const noexcept override;

    private:
        std::string _message;
    };
}

#endif
