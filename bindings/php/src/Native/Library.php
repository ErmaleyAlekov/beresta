<?php

declare(strict_types=1);

namespace Beresta\Native;

use Beresta\LibraryException;
use FFI;

/**
 * Единственный объект FFI с 71 функцией Си-интерфейса (паттерн Singleton).
 *
 * Объект создаётся лениво, при первом обращении, и живёт до конца процесса:
 * повторный FFI::cdef заново разбирал бы заголовок и искал бы символы.
 * Источники по очереди:
 *   1. область FFI "BERESTA", заранее загруженная через opcache.preload
 *      (Beresta::preload) — единственный путь для PHP-FPM / Apache при
 *      настройке по умолчанию ffi.enable=preload;
 *   2. FFI::cdef с заголовком resources/beresta.h и файлом библиотеки,
 *      найденным LibraryLocator, — CLI или ffi.enable=1.
 * Все символы ищутся при загрузке, поэтому несовпадение версий библиотеки
 * и заголовка видно сразу, а не на первом вызове отсутствующей функции.
 *
 * The only FFI object with the 71 functions of the C interface (the Singleton pattern).
 *
 * The object is created lazily, on the first access, and lives until the process
 * ends: a repeated FFI::cdef would parse the header and look up the symbols again.
 * The sources in turn:
 *   1. the "BERESTA" FFI scope loaded in advance through opcache.preload
 *      (Beresta::preload) — the only way for PHP-FPM / Apache with the default
 *      setting ffi.enable=preload;
 *   2. FFI::cdef with the resources/beresta.h header and the library file found
 *      by LibraryLocator — CLI or ffi.enable=1.
 * All symbols are looked up at load time, so a mismatch between the library and
 * header versions shows up at once, not on the first call of a missing function.
 */
final class Library
{
    /**
     * Имя области FFI для opcache.preload (FFI::scope).
     * The FFI scope name for opcache.preload (FFI::scope).
     */
    public const SCOPE = 'BERESTA';

    /**
     * Загруженная библиотека или null, пока к ней не обращались.
     * The loaded library or null until it is accessed.
     */
    private static ?FFI $ffi = null;

    /**
     * Объект FFI для вызова функций beresta_* (загружается один раз).
     * The FFI object for calling the beresta_* functions (loaded once).
     */
    public static function ffi(): FFI
    {
        if (self::$ffi === null) {
            self::$ffi = self::load();
        }
        return self::$ffi;
    }

    /**
     * Загрузка: сначала область preload, затем кандидаты LibraryLocator.
     * Loading: the preload scope first, then the LibraryLocator candidates.
     */
    private static function load(): FFI
    {
        if (!extension_loaded('ffi')) {
            throw LibraryException::ffiMissing();
        }
        $scoped = self::fromScope();
        if ($scoped !== null) {
            return $scoped;
        }
        self::assertCdefAllowed();
        $header = LibraryLocator::headerText();
        $tried = [];
        foreach (LibraryLocator::candidates() as $candidate) {
            $loaded = self::open($header, $candidate, $tried);
            if ($loaded !== null) {
                return $loaded;
            }
        }
        throw LibraryException::notFound(Platform::name(), $tried);
    }

    /**
     * Область "BERESTA" из opcache.preload или null, если её не загружали.
     * The "BERESTA" scope from opcache.preload or null if it was not loaded.
     */
    private static function fromScope(): ?FFI
    {
        try {
            return FFI::scope(self::SCOPE);
        } catch (FFI\Exception) {
            return null;
        }
    }

    /**
     * FFI::cdef разрешён при ffi.enable=1, а при ffi.enable=preload — только в CLI.
     * FFI::cdef is allowed with ffi.enable=1, and with ffi.enable=preload only in CLI.
     */
    private static function assertCdefAllowed(): void
    {
        $mode = strtolower((string) ini_get('ffi.enable'));
        if (in_array($mode, ['1', 'on', 'true', 'yes'], true)) {
            return;
        }
        if ($mode === 'preload' && PHP_SAPI === 'cli') {
            return;
        }
        throw LibraryException::ffiRestricted($mode);
    }

    /**
     * Попытка загрузить один кандидат; причина неудачи копится в $tried.
     * An attempt to load one candidate; the failure reason is collected in $tried.
     *
     * @param list<string> $tried
     */
    private static function open(string $header, string $candidate, array &$tried): ?FFI
    {
        if (LibraryLocator::isPath($candidate) && !is_file($candidate)) {
            $tried[] = $candidate;
            return null;
        }
        try {
            return FFI::cdef($header, $candidate);
        } catch (FFI\Exception $error) {
            $tried[] = $candidate . ' (' . $error->getMessage() . ')';
            return null;
        }
    }
}
