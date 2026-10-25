<?php

declare(strict_types=1);

namespace BerestaBench;

/**
 * Замер времени и памяти: медиана из RUNS прогонов после одного прогревочного.
 * Measuring time and memory: the median of RUNS runs after one warm-up run.
 */
final class Measure
{
    public const RUNS = 5;

    /**
     * Медиана времени операции в миллисекундах; результат сразу отпускается
     * (документ освобождается подсчётом ссылок до следующего прогона).
     *
     * The median operation time in milliseconds; the result is released at once
     * (the document is freed by reference counting before the next run).
     */
    public static function medianMs(callable $operation, mixed $argument): float
    {
        $times = [];
        for ($run = 0; $run <= self::RUNS; $run++) {
            $start = hrtime(true);
            $result = $operation($argument);
            $elapsed = (hrtime(true) - $start) / 1e6;
            unset($result);
            if ($run > 0) {
                $times[] = $elapsed;
            }
        }
        sort($times);
        return $times[intdiv(count($times), 2)];
    }

    /**
     * Время в мс или, если задан объём в байтах, скорость в МБ/с.
     * The time in ms or, if a size in bytes is given, the speed in MB/s.
     */
    public static function timed(callable $operation, mixed $argument, int $bytes = 0): string
    {
        $milliseconds = self::medianMs($operation, $argument);
        if ($bytes > 0) {
            return sprintf('%.0f', $bytes / 1e6 / $milliseconds * 1000);
        }
        return sprintf('%.1f', $milliseconds);
    }

    /**
     * VmRSS процесса в байтах (Linux); 0 — недоступно.
     * The process VmRSS in bytes (Linux); 0 means unavailable.
     */
    public static function residentBytes(): int
    {
        $status = @file_get_contents('/proc/self/status');
        if ($status === false || preg_match('/^VmRSS:\s+(\d+)/m', $status, $match) !== 1) {
            return 0;
        }
        return (int) $match[1] * 1024;
    }

    /**
     * Прирост памяти процесса после разбора — и объекты PHP, и нативная
     * память библиотеки. Каждая библиотека меряется в своём процессе.
     *
     * The process memory growth after parsing — both PHP objects and the native
     * memory of the library. Every library is measured in its own process.
     */
    public static function memoryLine(Engine $engine, string $xml): string
    {
        gc_collect_cycles();
        $before = self::residentBytes();
        $document = $engine->parse($xml);
        gc_collect_cycles();
        $grown = self::residentBytes() - $before;
        $line = sprintf('| %s | %d MB (×%.1f) |', $engine->name, intdiv($grown, 1000000), $grown / strlen($xml));
        $engine->walk($document);
        return $line;
    }
}
