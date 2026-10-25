<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Diagnostic;
use Beresta\Native\Calls;
use Beresta\Native\Library;
use Beresta\Native\Strings;
use FFI\CData;

/**
 * Чтение списка нарушений в массив Diagnostic и его освобождение.
 * Reading a violation list into a Diagnostic array and freeing it.
 *
 * @internal
 */
final class DiagnosticsReader
{
    /**
     * Все нарушения списка; сам список освобождается в любом случае.
     * All violations of the list; the list itself is freed in any case.
     *
     * @return list<Diagnostic>
     */
    public static function take(CData $diagnostics): array
    {
        $ffi = Library::ffi();
        try {
            $count = Calls::checkInt($ffi->beresta_diagnostics_count($diagnostics));
            $result = [];
            for ($index = 0; $index < $count; $index++) {
                $result[] = self::at($diagnostics, $index);
            }
            return $result;
        } finally {
            $ffi->beresta_diagnostics_free($diagnostics);
        }
    }

    /**
     * Одно нарушение: текст, строка и столбец (0 — место неизвестно).
     * One violation: the text, the line and the column (0 means the place is unknown).
     */
    private static function at(CData $diagnostics, int $index): Diagnostic
    {
        $ffi = Library::ffi();
        $message = Strings::take($ffi->beresta_diagnostics_message($diagnostics, $index)) ?? '';
        $line = Calls::checkInt($ffi->beresta_diagnostics_line($diagnostics, $index));
        $column = Calls::checkInt($ffi->beresta_diagnostics_column($diagnostics, $index));
        return new Diagnostic($message, $line, $column);
    }
}
