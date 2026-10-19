#include "io/mapped_file.hpp"
#include "io/file_io.hpp"

namespace beresta::io
{
    /*
    ** Сначала пробуем отобразить. Неудача — не ошибка: файл читается
    ** в буфер, и только если не удалось и это, readFile бросает
    ** DomException(IO_ERROR) с путём к файлу.
    **
    ** First we try to map. A failure is not an error: the file is read
    ** into a buffer, and only if that fails too, readFile throws
    ** DomException(IO_ERROR) with the file path.
    */
    MappedFile::MappedFile(const std::filesystem::path& path)
        : _mapping{nullptr, 0, nullptr}, _mapped(false)
    {
        _mapped = detail::mapFile(path, _mapping);
        if (!_mapped)
            _fallback = readFile(path);
    }

    // Отображение снимается здесь, буфер освобождается сам
    // The mapping is removed here, the buffer is freed by itself
    MappedFile::~MappedFile()
    {
        if (_mapped)
            detail::unmapFile(_mapping);
    }

    // Байты отображения или запасного буфера
    // The bytes of the mapping or of the fallback buffer
    std::string_view MappedFile::getBytes() const noexcept
    {
        if (_mapped)
            return std::string_view(_mapping.data, _mapping.size);
        return _fallback;
    }

    bool MappedFile::isMapped() const noexcept
    {
        return _mapped;
    }
}
