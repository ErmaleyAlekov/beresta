<?php

declare(strict_types=1);

namespace Beresta\Native;

use Beresta\LibraryException;

/**
 * Поиск динамической библиотеки «Берёсты» и заголовка для FFI.
 *
 * Порядок поиска (первый загрузившийся файл побеждает):
 *   1. переменная окружения BERESTA_LIBRARY — явный путь к файлу;
 *   2. папка пакета native/<os>-<arch>/ — туда бинарник кладут
 *      Makefile (make PHP=1) и CMake (-DBERESTA_BUILD_PHP=ON);
 *   3. системный поиск по одному имени файла (dlopen / LoadLibrary):
 *      библиотека, установленная в систему (/usr/lib, PATH и т. п.).
 *
 * Finding the Beresta shared library and the header for FFI.
 *
 * Lookup order (the first file that loads wins):
 *   1. the BERESTA_LIBRARY environment variable — an explicit file path;
 *   2. the package folder native/<os>-<arch>/ — the binary is put there by
 *      the Makefile (make PHP=1) and CMake (-DBERESTA_BUILD_PHP=ON);
 *   3. the system lookup by the bare file name (dlopen / LoadLibrary):
 *      a library installed in the system (/usr/lib, PATH and so on).
 */
final class LibraryLocator
{
    /**
     * Кандидаты в порядке приоритета; несуществующие пути отбрасывает загрузчик.
     * Candidates in priority order; the loader drops paths that do not exist.
     *
     * @return list<string>
     */
    public static function candidates(): array
    {
        $result = [];
        $explicit = getenv('BERESTA_LIBRARY');
        if (is_string($explicit) && $explicit !== '') {
            $result[] = $explicit;
        }
        $result[] = self::packageRoot() . '/native/' . Platform::name() . '/' . Platform::libraryFileName();
        $result[] = Platform::libraryFileName();
        return $result;
    }

    /**
     * Первый существующий файл-кандидат, иначе имя для системного поиска.
     * The first existing candidate file, otherwise the name for the system lookup.
     */
    public static function firstExisting(): string
    {
        foreach (self::candidates() as $candidate) {
            if (self::isPath($candidate) && is_file($candidate)) {
                return $candidate;
            }
        }
        return Platform::libraryFileName();
    }

    /**
     * true — кандидат задан путём, а не голым именем для системного поиска.
     * true means the candidate is a path, not a bare name for the system lookup.
     */
    public static function isPath(string $candidate): bool
    {
        return str_contains($candidate, '/') || str_contains($candidate, '\\');
    }

    /**
     * Текст заголовка resources/beresta.h для FFI::cdef.
     * The text of the resources/beresta.h header for FFI::cdef.
     */
    public static function headerText(): string
    {
        $text = file_get_contents(self::packageRoot() . '/resources/beresta.h');
        if ($text === false) {
            throw LibraryException::headerMissing(self::packageRoot() . '/resources/beresta.h');
        }
        return $text;
    }

    /**
     * Корень пакета: на две папки выше src/Native.
     * The package root: two folders above src/Native.
     */
    private static function packageRoot(): string
    {
        return dirname(__DIR__, 2);
    }
}
