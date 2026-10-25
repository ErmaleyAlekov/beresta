<?php

declare(strict_types=1);

namespace Beresta\Native;

use FFI;
use FFI\CData;

/**
 * Строки на границе PHP и Си: владение и преобразование.
 *
 * PHP FFI сам превращает возвращаемый const char* в строку PHP — это
 * заимствованные строки библиотеки. Строку char* он оставляет указателем
 * (FFI\CData): её нужно скопировать и освободить через beresta_string_free,
 * иначе память утечёт. Оба случая обрабатываются здесь.
 *
 * Strings on the border of PHP and C: ownership and conversion.
 *
 * PHP FFI itself turns a returned const char* into a PHP string — these are the
 * borrowed strings of the library. A char* string stays a pointer (FFI\CData):
 * it must be copied and freed through beresta_string_free, otherwise the memory
 * leaks. Both cases are handled here.
 */
final class Strings
{
    /**
     * Строка char*, принадлежащая вызывающему: копия в PHP, затем освобождение.
     * NULL — нет значения (или ошибка, тогда исключение).
     *
     * A char* string owned by the caller: a copy into PHP, then freeing.
     * NULL means no value (or an error, then an exception).
     */
    public static function take(?CData $pointer): ?string
    {
        if ($pointer === null) {
            Calls::checkHandle(null);
            return null;
        }
        try {
            return FFI::string($pointer);
        } finally {
            Library::ffi()->beresta_string_free($pointer);
        }
    }

    /**
     * Заимствованная строка const char*: уже строка PHP или, на всякий
     * случай, указатель; NULL — пустая строка. Освобождать её нельзя.
     *
     * A borrowed const char* string: already a PHP string or, just in case,
     * a pointer; NULL is an empty string. It must not be freed.
     */
    public static function borrowed(mixed $value): string
    {
        if (is_string($value)) {
            return $value;
        }
        if ($value instanceof CData) {
            return FFI::string($value);
        }
        return '';
    }
}
