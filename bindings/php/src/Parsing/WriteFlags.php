<?php

declare(strict_types=1);

namespace Beresta;

/**
 * Флаги вывода (битовая маска), как BERESTA_WRITE_*.
 *
 *     $document->save('order.xml', WriteFlags::PRETTY | WriteFlags::DECLARATION);
 *
 * Output flags (a bit mask), like BERESTA_WRITE_*.
 */
final class WriteFlags
{
    /** Компактно, без лишних пробелов. / Compact, without extra spaces. */
    public const DEFAULT = 0;

    /** Отступы и переводы строк. / Indentation and line breaks. */
    public const PRETTY = 1 << 0;

    /** Всегда писать <?xml ...?>. / Always write <?xml ...?>. */
    public const DECLARATION = 1 << 1;

    /**
     * Экземпляры не нужны: только константы. / No instances are needed: only constants.
     */
    private function __construct()
    {
    }
}
