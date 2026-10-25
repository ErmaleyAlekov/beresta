<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Diagnostic;
use Beresta\Document;
use Beresta\Native\Calls;
use Beresta\Native\Handles;
use Beresta\Native\Library;
use Beresta\ParseFlags;

/**
 * Трейт схемы: проверка готового дерева и потоковая проверка без дерева.
 *
 * Нарушение схемы — не исключение, а элемент списка Diagnostic: пустой
 * список значит «документ корректен». Синтаксическая ошибка XML в потоковой
 * проверке — исключение BerestaException PARSE_ERROR.
 *
 * A schema trait: validating a ready tree and streaming validation without a tree.
 *
 * A schema violation is not an exception but an item of the Diagnostic list: an
 * empty list means "the document is valid". An XML syntax error in streaming
 * validation is a BerestaException PARSE_ERROR.
 */
trait SchemaValidation
{
    /**
     * Все нарушения за один проход по готовому дереву. Строка и столбец
     * известны, если документ разобран с ParseFlags::TRACK_LOCATIONS.
     *
     * All violations in one pass over a ready tree. The line and column are known
     * if the document was parsed with ParseFlags::TRACK_LOCATIONS.
     *
     * @return list<Diagnostic>
     */
    public function validate(Document $document): array
    {
        $slot = Handles::slot();
        Calls::check(Library::ffi()->beresta_schema_validate($this->handle(), $document->handle(), Handles::out($slot)));
        return DiagnosticsReader::take($slot);
    }

    /**
     * Потоковая проверка строки без построения дерева.
     * Streaming validation of a string without building a tree.
     *
     * @return list<Diagnostic>
     */
    public function validateString(string $xml, int $flags = ParseFlags::DEFAULT): array
    {
        $slot = Handles::slot();
        $status = Library::ffi()->beresta_schema_validate_bytes(
            $xml, strlen($xml), $flags, $this->handle(), Handles::out($slot));
        Calls::check($status);
        return DiagnosticsReader::take($slot);
    }

    /**
     * Потоковая проверка файла: память — O(глубины документа), файл
     * отображается в память, поэтому так проверяются выгрузки в гигабайты.
     *
     * Streaming validation of a file: the memory is O(document depth), the file is
     * mapped into memory, so exports of gigabytes are validated this way.
     *
     * @return list<Diagnostic>
     */
    public function validateFile(string $path, int $flags = ParseFlags::DEFAULT): array
    {
        $slot = Handles::slot();
        Calls::check(Library::ffi()->beresta_schema_validate_file($path, $flags, $this->handle(), Handles::out($slot)));
        return DiagnosticsReader::take($slot);
    }
}
