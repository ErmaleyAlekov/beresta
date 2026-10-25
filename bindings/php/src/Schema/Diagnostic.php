<?php

declare(strict_types=1);

namespace Beresta;

/**
 * Одно нарушение схемы: текст на текущем языке и место в исходнике.
 * line и column — с единицы; 0 — место неизвестно (документ разобран без
 * ParseFlags::TRACK_LOCATIONS).
 *
 *     echo $problem;                 // "12:5: …"
 *     echo $problem->message;
 *
 * One schema violation: the text in the current language and the place in the
 * source. line and column start at one; 0 means the place is unknown (the document
 * was parsed without ParseFlags::TRACK_LOCATIONS).
 */
final readonly class Diagnostic implements \Stringable
{
    public function __construct(
        public string $message,
        public int $line,
        public int $column,
    ) {
    }

    /**
     * "строка:столбец: текст" или просто текст, если место неизвестно.
     * "line:column: text" or just the text if the place is unknown.
     */
    public function __toString(): string
    {
        if ($this->line === 0) {
            return $this->message;
        }
        return $this->line . ':' . $this->column . ': ' . $this->message;
    }
}
