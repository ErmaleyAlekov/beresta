#include "io/file_io.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>

namespace beresta::io
{
    // Файл открывается в двоичном режиме и читается целиком через буфер потока
    // The file is opened in binary mode and read whole through the stream buffer
    std::string readFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);

        if (!file)
            throw DomException(DomErrorCode::IO_ERROR,
                i18n::format(i18n::Message::FILE_OPEN_READ, {path.u8string()}));
        return readStream(file);
    }

    // Запись с проверкой: заполненный диск или отказ в доступе не пройдут незамеченными
    // Writing with a check: a full disk or an access denial will not go unnoticed
    void writeFile(const std::filesystem::path& path, std::string_view bytes)
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);

        if (!file)
            throw DomException(DomErrorCode::IO_ERROR,
                i18n::format(i18n::Message::FILE_OPEN_WRITE, {path.u8string()}));
        writeStream(file, bytes);
    }

    // rdbuf() копирует поток за одну операцию без построчного чтения
    // rdbuf() copies the stream in one operation without reading line by line
    std::string readStream(std::istream& stream)
    {
        std::ostringstream buffer;

        buffer << stream.rdbuf();
        if (stream.bad())
            throw DomException(DomErrorCode::IO_ERROR, i18n::format(i18n::Message::STREAM_READ_FAILED));
        return buffer.str();
    }

    // Запись и принудительный сброс буфера с проверкой состояния
    // Writing and a forced buffer flush with a state check
    void writeStream(std::ostream& stream, std::string_view bytes)
    {
        stream.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        stream.flush();
        if (!stream)
            throw DomException(DomErrorCode::IO_ERROR, i18n::format(i18n::Message::STREAM_WRITE_FAILED));
    }
}
