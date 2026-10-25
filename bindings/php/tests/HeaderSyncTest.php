<?php

declare(strict_types=1);

namespace Beresta\Tests;

use Beresta\Beresta;
use Beresta\Native\Library;
use Beresta\Tests\Support\TestCase;

/**
 * Заголовок FFI resources/beresta.h совпадает с include/capi/beresta_c.h.
 *
 * Заголовок PHP повторяет Си-интерфейс вручную (у FFI нет препроцессора),
 * поэтому новая функция beresta_c.h без строки здесь была бы недоступна из
 * PHP. Тест сравнивает множества имён функций; в пакете из Composer файла
 * beresta_c.h нет — тогда проверяется только число функций.
 *
 * The FFI header resources/beresta.h matches include/capi/beresta_c.h.
 *
 * The PHP header repeats the C interface by hand (FFI has no preprocessor), so a
 * new beresta_c.h function without a line here would be unavailable from PHP. The
 * test compares the sets of function names; a package from Composer has no
 * beresta_c.h — then only the number of functions is checked.
 */
final class HeaderSyncTest extends TestCase
{
    /**
     * Те же 71 функция, что и в Си-интерфейсе. / The same 71 functions as in the C interface.
     */
    public function testHeaderDeclaresAllFunctions(): void
    {
        $php = self::functionNames(__DIR__ . '/../resources/beresta.h', '');
        $this->assertSame(71, count($php));
        $original = __DIR__ . '/../../../include/capi/beresta_c.h';
        if (is_file($original)) {
            $this->assertSame(self::functionNames($original, 'BERESTA_API '), $php);
        }
    }

    /**
     * FFI::cdef нашёл все символы в библиотеке, и версия читается.
     * FFI::cdef found all symbols in the library, and the version is readable.
     */
    public function testLibraryLoads(): void
    {
        Library::ffi();
        $this->assertTrue(preg_match('/^\d+\.\d+\.\d+$/', Beresta::version()) === 1);
    }

    /**
     * Отсортированные имена beresta_* из объявлений функций файла.
     * The sorted beresta_* names from the function declarations of the file.
     *
     * @return list<string>
     */
    private static function functionNames(string $path, string $prefix): array
    {
        $pattern = '/^' . preg_quote($prefix, '/') . '[\w\s\*]*?\b(beresta_\w+)\s*\(/m';
        preg_match_all($pattern, (string) file_get_contents($path), $matches);
        $names = array_values(array_unique($matches[1]));
        sort($names);
        return $names;
    }
}
