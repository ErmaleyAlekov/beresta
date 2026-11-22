<?php

declare(strict_types=1);

namespace BerestaBench;

/** Чтение входа бенчмарка. / Reading benchmark input. */
final class BenchInput
{
    public const READ_FAILED = 'Не удалось прочитать файл / Could not read file: ';
    public const INVALID_XML = 'Некорректный XML / Invalid XML';
    public const INVALID_XSD = 'Некорректная схема XSD / Invalid XSD schema: ';
    private const MEMORY_INDEX_ERROR = '--memory: нужен номер доступной библиотеки / expected an available library index';

    /** Чтение файла с диагностикой при ошибке. / Read a file with a diagnostic on failure. */
    public static function read(string $path): string
    {
        $contents = @file_get_contents($path);
        if ($contents === false) {
            throw new \RuntimeException(self::READ_FAILED . $path);
        }
        return $contents;
    }

    /** Индекс библиотеки для режима памяти. / The library index for memory mode. */
    public static function memoryIndex(array $arguments, int $count): int
    {
        $value = $arguments[3] ?? '';
        if (count($arguments) !== 4 || preg_match('/^[0-9]+$/D', $value) !== 1 || (int) $value >= $count) {
            throw new \RuntimeException(self::MEMORY_INDEX_ERROR);
        }
        return (int) $value;
    }
}
