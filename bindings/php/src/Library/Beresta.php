<?php

declare(strict_types=1);

namespace Beresta;

use Beresta\Native\Library;
use Beresta\Native\Platform;
use Beresta\Native\Preloader;
use Beresta\Native\Strings;

/**
 * Сведения о библиотеке и её загрузка для веб-сервера.
 *
 *     echo Beresta::version();    // 2.1.4
 *     echo Beresta::platform();   // linux-x86_64
 *
 * В opcache.preload (PHP-FPM, Apache с ffi.enable=preload):
 *
 *     require __DIR__ . '/vendor/autoload.php';
 *     Beresta\Beresta::preload();
 *
 * Information about the library and loading it for a web server.
 * In opcache.preload (PHP-FPM, Apache with ffi.enable=preload) see the example above.
 */
final class Beresta
{
    /**
     * Версия нативной библиотеки, например "2.1.4".
     * The native library version, for example "2.1.4".
     */
    public static function version(): string
    {
        return Strings::borrowed(Library::ffi()->beresta_version());
    }

    /**
     * Платформа вида windows-x86_64 — имя папки в native/.
     * The platform like windows-x86_64 — the folder name in native/.
     */
    public static function platform(): string
    {
        return Platform::name();
    }

    /**
     * Регистрация области FFI "BERESTA" из скрипта opcache.preload;
     * $library === null — путь ищется так же, как при обычной загрузке.
     *
     * Registers the "BERESTA" FFI scope from the opcache.preload script;
     * $library === null means the path is looked up as in ordinary loading.
     */
    public static function preload(?string $library = null): void
    {
        Preloader::preload($library);
    }
}
