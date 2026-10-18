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
    **
    ** Reading and writing "raw" bytes. Files are opened in binary mode:
    ** no line ending conversions by the OS — the parser / serializer itself
    ** handles the encoding and normalization.
    ** Paths are std::filesystem::path, so Cyrillic file names work
    ** correctly on Windows too. Errors are DomException(IO_ERROR).
    */

    // Читает весь файл в строку
    // Reads the whole file into a string
    std::string readFile(const std::filesystem::path& path);

    // Перезаписывает файл указанными байтами
    // Overwrites the file with the given bytes
    void writeFile(const std::filesystem::path& path, std::string_view bytes);

    // Читает поток до конца
    // Reads a stream to the end
    std::string readStream(std::istream& stream);

    // Пишет байты в поток
    // Writes bytes to a stream
    void writeStream(std::ostream& stream, std::string_view bytes);
}

#endif
