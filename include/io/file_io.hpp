#ifndef FILE_IO_HPP
#define FILE_IO_HPP

#include <filesystem>
#include <iosfwd>
#include <string>
#include <string_view>

namespace beresta::io
{
    /*
    ** Чтение и запись «сырых» байтов. Файлы открываются в двоичном режиме:
    ** никаких преобразований переводов строк средствами ОС — кодировкой
    ** и нормализацией занимается сам парсер / сериализатор.
    ** Пути — std::filesystem::path, поэтому кириллические имена файлов
    ** корректно работают и на Windows. Ошибки — DomException(IO_ERROR).
    */

    // Читает весь файл в строку
    std::string readFile(const std::filesystem::path& path);

    // Перезаписывает файл указанными байтами
    void writeFile(const std::filesystem::path& path, std::string_view bytes);

    // Читает поток до конца
    std::string readStream(std::istream& stream);

    // Пишет байты в поток
    void writeStream(std::ostream& stream, std::string_view bytes);
}

#endif
