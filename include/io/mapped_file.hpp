#ifndef MAPPED_FILE_HPP
#define MAPPED_FILE_HPP

#include "io/internal/file_mapping.hpp"

#include <filesystem>
#include <string>
#include <string_view>

namespace beresta::io
{
    /*
    ** MappedFile — байты файла только для чтения, отображённые в память
    ** (mmap на Linux и macOS, MapViewOfFile на Windows; паттерн RAII:
    ** отображение снимается в деструкторе).
    **
    ** Зачем: readFile копирует файл в std::string через ostringstream, и на
    ** пике в памяти лежат два-три размера файла. Отображённые страницы
    ** читаются ОС по мере обращения и принадлежат файловому кешу: при нехватке
    ** памяти ОС просто выгружает уже пройденные, не трогая файл подкачки.
    ** Поэтому SAX-разбор и потоковая проверка по XSD гигабайтного файла
    ** держат в собственной памяти процесса только O(глубины документа).
    **
    ** Если отобразить нельзя (пустой файл, канал, устройство, WebAssembly
    ** без файловой системы), файл читается обычным образом — вызывающему
    ** коду разница не видна. Файл нельзя укорачивать, пока объект жив:
    ** обращение к исчезнувшей странице на Unix завершает процесс (SIGBUS).
    **
    ** MappedFile — the read-only bytes of a file mapped into memory
    ** (mmap on Linux and macOS, MapViewOfFile on Windows; the RAII pattern:
    ** the mapping is removed in the destructor).
    **
    ** Why: readFile copies the file into std::string through ostringstream, and at
    ** the peak two or three file sizes lie in memory. Mapped pages are read by the
    ** OS on access and belong to the file cache: when memory runs short, the OS
    ** simply drops the pages already passed without touching the swap file.
    ** That is why SAX parsing and streaming XSD validation of a gigabyte file
    ** keep only O(document depth) in the process's own memory.
    **
    ** If mapping is impossible (an empty file, a pipe, a device, WebAssembly
    ** without a file system), the file is read the ordinary way — the calling
    ** code sees no difference. The file must not be truncated while the object
    ** is alive: an access to a vanished page terminates the process on Unix (SIGBUS).
    */
    class MappedFile
    {
    public:
        // Открывает и отображает файл; DomException(IO_ERROR), если его не прочитать
        // Opens and maps the file; DomException(IO_ERROR) if it cannot be read
        explicit MappedFile(const std::filesystem::path& path);
        ~MappedFile();

        MappedFile(const MappedFile&) = delete;
        MappedFile& operator=(const MappedFile&) = delete;

        // Все байты файла; живут, пока жив объект
        // All bytes of the file; they live as long as the object does
        std::string_view getBytes() const noexcept;

        // true — байты отображены, false — прочитаны в буфер
        // true — the bytes are mapped, false — they were read into a buffer
        bool isMapped() const noexcept;

    private:
        detail::FileMapping _mapping;
        bool _mapped;
        std::string _fallback;
    };
}

#endif
