#if !defined(_WIN32)

#include "io/internal/file_mapping.hpp"

#if !defined(__EMSCRIPTEN__)
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace beresta::io::detail
{
#if defined(__EMSCRIPTEN__)
    // WebAssembly собирается без файловой системы: отображать нечего
    // WebAssembly is built without a file system: there is nothing to map
    bool mapFile(const std::filesystem::path&, FileMapping&) noexcept
    {
        return false;
    }

    void unmapFile(const FileMapping&) noexcept
    {
    }
#else
    /*
    ** Linux и macOS: open + fstat + mmap. Отображаются только обычные
    ** файлы ненулевой длины: канал или устройство нельзя отобразить, а
    ** mmap длины 0 — ошибка. MAP_PRIVATE + PROT_READ: процесс видит
    ** снимок только для чтения. madvise(MADV_SEQUENTIAL) подсказывает ядру
    ** читать вперёд и раньше выгружать пройденное — разбор идёт от начала
    ** к концу. Дескриптор закрывается сразу: отображение от него не зависит.
    **
    ** Linux and macOS: open + fstat + mmap. Only regular files of non-zero
    ** length are mapped: a pipe or a device cannot be mapped, and an mmap of
    ** length 0 is an error. MAP_PRIVATE + PROT_READ: the process sees a
    ** read-only snapshot. madvise(MADV_SEQUENTIAL) hints the kernel to read
    ** ahead and to drop the passed pages earlier — parsing goes from the start
    ** to the end. The descriptor is closed at once: the mapping does not depend on it.
    */
    bool mapFile(const std::filesystem::path& path, FileMapping& mapping) noexcept
    {
        int file = open(path.c_str(), O_RDONLY | O_CLOEXEC);
        if (file < 0)
            return false;
        struct stat status;
        void* data = MAP_FAILED;
        if (fstat(file, &status) == 0 && S_ISREG(status.st_mode) && status.st_size > 0
            && static_cast<unsigned long long>(status.st_size) <= static_cast<std::size_t>(-1))
            data = mmap(nullptr, static_cast<std::size_t>(status.st_size), PROT_READ, MAP_PRIVATE, file, 0);
        close(file);
        if (data == MAP_FAILED)
            return false;
        madvise(data, static_cast<std::size_t>(status.st_size), MADV_SEQUENTIAL);
        mapping = FileMapping{static_cast<const char*>(data), static_cast<std::size_t>(status.st_size), nullptr};
        return true;
    }

    // munmap ждёт неконстантный указатель / munmap expects a non-const pointer
    void unmapFile(const FileMapping& mapping) noexcept
    {
        munmap(const_cast<char*>(mapping.data), mapping.size);
    }
#endif
}

#endif
