#ifndef FILE_MAPPING_HPP
#define FILE_MAPPING_HPP

#include <cstddef>
#include <filesystem>

namespace beresta::io::detail
{
    /*
    ** Платформенная часть MappedFile. Реализации лежат в отдельных файлах
    ** (file_mapping_windows.cpp и file_mapping_posix.cpp), и каждый
    ** компилируется только на своей ОС: в одном файле нет развилок #ifdef
    ** внутри функций, а MappedFile не знает ни одного системного вызова.
    **
    ** The platform part of MappedFile. The implementations are in separate files
    ** (file_mapping_windows.cpp and file_mapping_posix.cpp), and each one
    ** is compiled only on its own OS: there are no #ifdef branches inside the
    ** functions, and MappedFile does not know a single system call.
    */
    struct FileMapping
    {
        const char* data;      // начало отображения / the start of the mapping
        std::size_t size;      // длина в байтах / the length in bytes
        void* system;          // HANDLE отображения на Windows, на Unix не нужен
                               // the mapping HANDLE on Windows, not needed on Unix
    };

    // Отображение всего файла; false — отобразить нельзя, нужно обычное чтение
    // Maps the whole file; false — it cannot be mapped, ordinary reading is needed
    bool mapFile(const std::filesystem::path& path, FileMapping& mapping) noexcept;

    // Снятие отображения, созданного mapFile
    // Removes a mapping created by mapFile
    void unmapFile(const FileMapping& mapping) noexcept;
}

#endif
