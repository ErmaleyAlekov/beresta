<?php

declare(strict_types=1);

namespace Beresta;

/**
 * Флаги разбора (битовая маска, объединяются через |), как BERESTA_PARSE_*.
 *
 * Константы класса, а не перечисление: перечисления PHP нельзя объединять
 * побитово, а маска — привычный для PHP вид (как JSON_* у json_encode).
 *
 *     new Parser(ParseFlags::PRESERVE_WHITESPACE | ParseFlags::TRACK_LOCATIONS);
 *
 * Parsing flags (a bit mask, combined with |), like BERESTA_PARSE_*.
 *
 * Class constants, not an enum: PHP enums cannot be combined bitwise, and a mask
 * is the usual form in PHP (like JSON_* of json_encode).
 */
final class ParseFlags
{
    /** Разбор по умолчанию. / The default parsing. */
    public const DEFAULT = 0;

    /** Хранить пробельный текст между тегами. / Keep whitespace text between tags. */
    public const PRESERVE_WHITESPACE = 1 << 0;

    /** Не создавать узлы комментариев. / Do not create comment nodes. */
    public const DISCARD_COMMENTS = 1 << 1;

    /** CDATA превращать в обычный текст. / Turn CDATA into ordinary text. */
    public const COALESCE_CDATA = 1 << 2;

    /** Проверять документ по DTD. / Validate the document against the DTD. */
    public const VALIDATE_DTD = 1 << 3;

    /** Отключить обработку xmlns. / Disable xmlns processing. */
    public const NO_NAMESPACES = 1 << 4;

    /** Выполнять xi:include. / Perform xi:include. */
    public const XINCLUDE = 1 << 5;

    /**
     * Разрешить внешние DTD, сущности и XInclude из каталога разбираемого файла.
     * Allow external DTDs, entities and XInclude from the directory of the parsed file.
     */
    public const FILE_RESOURCES = 1 << 6;

    /**
     * Запоминать строку и столбец узлов: тогда Schema::validate даёт места
     * нарушений (разбор примерно на 8 % медленнее).
     * Remember the line and column of nodes: then Schema::validate gives the
     * places of violations (parsing is about 8 % slower).
     */
    public const TRACK_LOCATIONS = 1 << 7;

    /**
     * Экземпляры не нужны: только константы. / No instances are needed: only constants.
     */
    private function __construct()
    {
    }
}
