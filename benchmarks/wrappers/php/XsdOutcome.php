<?php

declare(strict_types=1);

namespace BerestaBench;

/** Представление результата XSD. / Presentation of an XSD validation result. */
final class XsdOutcome
{
    /** Сопоставимая отметка валидности. / Comparable validity indicator. */
    public static function validity(?int $errors): string
    {
        if ($errors === null) {
            return '—';
        }
        if ($errors === 0) {
            return 'yes';
        }
        return 'no';
    }

    /** Число ошибок libxml; предупреждения пропускаются. / Count libxml errors, excluding warnings. */
    public static function libxmlErrors(): int
    {
        $errors = 0;
        foreach (libxml_get_errors() as $entry) {
            if ($entry->level >= LIBXML_ERR_ERROR) {
                $errors++;
            }
        }
        return $errors;
    }
}
