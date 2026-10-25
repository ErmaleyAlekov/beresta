<?php

declare(strict_types=1);

namespace Beresta\Native;

use Beresta\LibraryException;
use FFI;

/**
 * Загрузка библиотеки в opcache.preload — для PHP-FPM и Apache.
 *
 * При ffi.enable=preload (значение по умолчанию) FFI::cdef в веб-запросе
 * запрещён: доступны только области FFI, загруженные скриптом preload через
 * FFI::load. FFI::load читает заголовок из файла и берёт путь к библиотеке
 * из строки #define FFI_LIB, поэтому заголовок с этими строками собирается
 * во временном файле: путь к пакету (vendor/) известен только во время
 * выполнения, а сам пакет может лежать в каталоге только для чтения.
 *
 * Loading the library in opcache.preload — for PHP-FPM and Apache.
 *
 * With ffi.enable=preload (the default value) FFI::cdef is forbidden in a web
 * request: only the FFI scopes loaded by the preload script through FFI::load are
 * available. FFI::load reads the header from a file and takes the library path
 * from the #define FFI_LIB line, so the header with these lines is assembled in
 * a temporary file: the package path (vendor/) is known only at run time, and
 * the package itself may live in a read-only directory.
 */
final class Preloader
{
    /**
     * Регистрация области "BERESTA"; $library === null — поиск LibraryLocator.
     * Registers the "BERESTA" scope; $library === null means the LibraryLocator lookup.
     */
    public static function preload(?string $library = null): void
    {
        if (!extension_loaded('ffi')) {
            throw LibraryException::ffiMissing();
        }
        $path = $library ?? LibraryLocator::firstExisting();
        $file = sys_get_temp_dir() . DIRECTORY_SEPARATOR . 'beresta_ffi_' . md5($path) . '.h';
        if (file_put_contents($file, self::scopedHeader($path)) === false) {
            throw LibraryException::preloadFailed($file);
        }
        if (FFI::load($file) === null) {
            throw LibraryException::preloadFailed($file);
        }
    }

    /**
     * Заголовок с именем области и путём к библиотеке в первых строках.
     * The header with the scope name and the library path in the first lines.
     */
    private static function scopedHeader(string $library): string
    {
        // FFI_LIB читается без обработки escape-последовательностей: путь как есть
        // FFI_LIB is read without processing escape sequences: the path as is
        return '#define FFI_SCOPE "' . Library::SCOPE . "\"\n"
            . '#define FFI_LIB "' . $library . "\"\n"
            . LibraryLocator::headerText();
    }
}
