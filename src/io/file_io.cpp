#include "io/file_io.hpp"
#include "exception/dom_exception.hpp"

#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>

namespace beresta::io
{
    // Файл открывается в двоичном режиме и читается целиком через буфер потока
    std::string readFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);

        if (!file)
            throw DomException(DomErrorCode::IO_ERROR,
                "не удалось открыть файл для чтения: " + path.u8string());
        return readStream(file);
    }

    // Запись с проверкой: заполненный диск или отказ в доступе не пройдут незамеченными
    void writeFile(const std::filesystem::path& path, std::string_view bytes)
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);

        if (!file)
            throw DomException(DomErrorCode::IO_ERROR,
                "не удалось открыть файл для записи: " + path.u8string());
        writeStream(file, bytes);
    }

    // rdbuf() копирует поток за одну операцию без построчного чтения
    std::string readStream(std::istream& stream)
    {
        std::ostringstream buffer;

        buffer << stream.rdbuf();
        if (stream.bad())
            throw DomException(DomErrorCode::IO_ERROR, "ошибка чтения из потока");
        return buffer.str();
    }

    // Запись и принудительный сброс буфера с проверкой состояния
    void writeStream(std::ostream& stream, std::string_view bytes)
    {
        stream.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        stream.flush();
        if (!stream)
            throw DomException(DomErrorCode::IO_ERROR, "ошибка записи в поток");
    }
}
