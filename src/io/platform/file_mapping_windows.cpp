#if defined(_WIN32)

#include "io/internal/file_mapping.hpp"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>

namespace beresta::io::detail
{
    /*
    ** Windows: файл -> объект отображения -> вид (view) на весь файл.
    ** Путь передаётся как wchar_t (path::c_str()), поэтому кириллица
    ** в имени работает. FILE_SHARE_READ не мешает другим читать файл.
    ** Сам файл можно закрыть сразу: объект отображения держит его открытым.
    ** Пустой файл отобразить нельзя (CreateFileMapping отказывает) —
    ** тогда false, и MappedFile читает его обычным способом.
    **
    ** Windows: a file -> a mapping object -> a view of the whole file.
    ** The path is passed as wchar_t (path::c_str()), so Cyrillic in the name
    ** works. FILE_SHARE_READ does not prevent others from reading the file.
    ** The file itself can be closed at once: the mapping object keeps it open.
    ** An empty file cannot be mapped (CreateFileMapping refuses) —
    ** then false, and MappedFile reads it the ordinary way.
    */
    bool mapFile(const std::filesystem::path& path, FileMapping& mapping) noexcept
    {
        HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (file == INVALID_HANDLE_VALUE)
            return false;
        LARGE_INTEGER size;
        HANDLE view = nullptr;
        size.QuadPart = 0;
        if (GetFileType(file) == FILE_TYPE_DISK && GetFileSizeEx(file, &size) && size.QuadPart > 0
            && static_cast<unsigned long long>(size.QuadPart) <= static_cast<SIZE_T>(-1))
            view = CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
        CloseHandle(file);
        if (view == nullptr)
            return false;
        void* data = MapViewOfFile(view, FILE_MAP_READ, 0, 0, 0);
        if (data == nullptr)
        {
            CloseHandle(view);
            return false;
        }
        mapping = FileMapping{static_cast<const char*>(data), static_cast<std::size_t>(size.QuadPart), view};
        return true;
    }

    // Сначала вид, затем объект отображения
    // First the view, then the mapping object
    void unmapFile(const FileMapping& mapping) noexcept
    {
        UnmapViewOfFile(mapping.data);
        CloseHandle(static_cast<HANDLE>(mapping.system));
    }
}

#endif
