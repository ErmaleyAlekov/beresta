<?php

declare(strict_types=1);

namespace Beresta;

/**
 * Библиотеку нельзя загрузить: нет расширения FFI, FFI запрещён настройкой
 * ffi.enable или файл библиотеки не найден.
 *
 * Тексты здесь записаны на двух языках прямо в коде — это единственное
 * исключение из правила каталога сообщений: каталог живёт в самой
 * библиотеке, а она как раз и не загрузилась.
 *
 * The library cannot be loaded: the FFI extension is missing, FFI is forbidden
 * by the ffi.enable setting, or the library file is not found.
 *
 * The texts here are written in two languages right in the code — the only
 * exception from the message catalog rule: the catalog lives in the library
 * itself, and that is exactly what failed to load.
 */
final class LibraryException extends \RuntimeException implements BerestaThrowable
{
    /**
     * Расширение ffi не подключено. / The ffi extension is not enabled.
     */
    public static function ffiMissing(): self
    {
        return new self('Берёста: нужно расширение PHP ffi (extension=ffi в php.ini). '
            . '/ Beresta: the PHP ffi extension is required (extension=ffi in php.ini).');
    }

    /**
     * FFI::cdef запрещён настройкой ffi.enable. / FFI::cdef is forbidden by ffi.enable.
     */
    public static function ffiRestricted(string $mode): self
    {
        return new self('Берёста: FFI запрещён настройкой ffi.enable=' . $mode
            . '. Задайте ffi.enable=1 или загрузите библиотеку в opcache.preload через Beresta::preload(). '
            . '/ Beresta: FFI is forbidden by the ffi.enable=' . $mode
            . ' setting. Set ffi.enable=1 or load the library in opcache.preload through Beresta::preload().');
    }

    /**
     * Ни один кандидат не загрузился. / None of the candidates loaded.
     *
     * @param list<string> $tried
     */
    public static function notFound(string $platform, array $tried): self
    {
        $paths = implode(', ', $tried);
        return new self('Берёста: динамическая библиотека не найдена для платформы ' . $platform
            . '. Соберите её с флагом PHP=1 (make) или -DBERESTA_BUILD_PHP=ON (CMake) либо задайте BERESTA_LIBRARY. '
            . 'Проверенные пути: ' . $paths
            . ' / Beresta: the native library was not found for the platform ' . $platform
            . '. Build it with PHP=1 (make) or -DBERESTA_BUILD_PHP=ON (CMake), or set BERESTA_LIBRARY. '
            . 'Paths tried: ' . $paths);
    }

    /**
     * Нет файла заголовка resources/beresta.h. / The resources/beresta.h header is missing.
     */
    public static function headerMissing(string $path): self
    {
        return new self('Берёста: не найден заголовок FFI ' . $path
            . ' / Beresta: the FFI header is not found ' . $path);
    }

    /**
     * FFI::load в opcache.preload не удался. / FFI::load in opcache.preload failed.
     */
    public static function preloadFailed(string $header): self
    {
        return new self('Берёста: не удалось загрузить область FFI из ' . $header
            . ' / Beresta: failed to load the FFI scope from ' . $header);
    }
}
