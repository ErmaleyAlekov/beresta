<?php

declare(strict_types=1);

namespace Beresta\Native;

/**
 * Имена ОС и архитектуры в том виде, как их понимают сборка и обёртки.
 *
 * Имена совпадают с CMake, Makefile и обёртками Java / Python:
 * windows/linux/macos и x86_64/aarch64/x86. Папка с бинарником внутри
 * пакета называется native/<os>-<arch>/, например native/windows-x86_64/.
 *
 * The OS and architecture names in the form the build and the bindings use.
 *
 * The names match CMake, the Makefile and the Java / Python bindings:
 * windows/linux/macos and x86_64/aarch64/x86. The folder with the binary inside
 * the package is named native/<os>-<arch>/, for example native/windows-x86_64/.
 */
final class Platform
{
    /**
     * Разные системы называют одну архитектуру по-разному.
     * Different systems name the same architecture differently.
     */
    private const ARCH_ALIASES = [
        'amd64' => 'x86_64', 'x86_64' => 'x86_64', 'x64' => 'x86_64',
        'arm64' => 'aarch64', 'aarch64' => 'aarch64',
        'i386' => 'x86', 'i686' => 'x86', 'x86' => 'x86',
    ];

    /**
     * Каноническое имя файла библиотеки на каждой ОС.
     * The canonical library file name on each OS.
     */
    private const LIBRARY_FILES = [
        'windows' => 'beresta.dll',
        'linux' => 'libberesta.so',
        'macos' => 'libberesta.dylib',
    ];

    /**
     * Имя платформы вида windows-x86_64 (имя папки в native/).
     * The platform name like windows-x86_64 (the folder name in native/).
     */
    public static function name(): string
    {
        return self::os() . '-' . self::arch();
    }

    /**
     * Имя ОС: windows, macos, linux (прочие — как их называет PHP, строчными).
     * The OS name: windows, macos, linux (others as PHP names them, in lower case).
     */
    public static function os(): string
    {
        if (PHP_OS_FAMILY === 'Windows') {
            return 'windows';
        }
        if (PHP_OS_FAMILY === 'Darwin') {
            return 'macos';
        }
        return strtolower(PHP_OS_FAMILY);
    }

    /**
     * Имя архитектуры процессора, на которой работает сам PHP.
     * The name of the processor architecture PHP itself runs on.
     */
    public static function arch(): string
    {
        $machine = strtolower(php_uname('m'));
        $arch = self::ARCH_ALIASES[$machine] ?? $machine;
        // 32-битный PHP на 64-битном процессоре загрузит только 32-битную библиотеку
        // A 32-bit PHP on a 64-bit processor can load only a 32-bit library
        if ($arch === 'x86_64' && PHP_INT_SIZE === 4) {
            $arch = 'x86';
        }
        return $arch;
    }

    /**
     * Имя файла библиотеки для текущей ОС.
     * The library file name for the current OS.
     */
    public static function libraryFileName(): string
    {
        return self::LIBRARY_FILES[self::os()] ?? 'libberesta.so';
    }
}
