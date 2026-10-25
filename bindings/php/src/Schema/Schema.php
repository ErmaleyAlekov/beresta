<?php

declare(strict_types=1);

namespace Beresta;

use Beresta\Internal\SchemaValidation;
use Beresta\Native\Calls;
use Beresta\Native\Handles;
use Beresta\Native\Library;
use FFI\CData;

/**
 * Загруженная XML Schema: после загрузки неизменяема, одну схему можно
 * проверять сколько угодно документов.
 *
 *     $schema = Schema::fromFile('order.xsd');
 *     foreach ($schema->validate($document) as $problem) {   // готовое дерево / a ready tree
 *         echo $problem, PHP_EOL;
 *     }
 *     $problems = $schema->validateFile('export.xml');      // поток без дерева / a stream without a tree
 *
 * Создаётся только фабричными методами fromString / fromFile.
 *
 * A loaded XML Schema: immutable after loading, one schema can validate any number
 * of documents. Created only by the fromString / fromFile factory methods.
 */
final class Schema
{
    use SchemaValidation;

    /**
     * Схема по дескриптору от библиотеки. / A schema from a library handle.
     */
    private function __construct(private readonly CData $pointer)
    {
    }

    /**
     * Схема из текста XSD (xs:include / xs:import здесь недоступны — нет каталога).
     * A schema from XSD text (xs:include / xs:import are unavailable here — there is no directory).
     */
    public static function fromString(string $xsd): self
    {
        $slot = Handles::slot();
        Calls::check(Library::ffi()->beresta_schema_load($xsd, strlen($xsd), Handles::out($slot)));
        return new self($slot);
    }

    /**
     * Схема из файла; xs:include / xs:import ищутся рядом с ним.
     * A schema from a file; xs:include / xs:import are looked up next to it.
     */
    public static function fromFile(string $path): self
    {
        $slot = Handles::slot();
        Calls::check(Library::ffi()->beresta_schema_load_file($path, Handles::out($slot)));
        return new self($slot);
    }

    /**
     * Освобождение схемы, когда на неё не осталось ссылок (в том числе у Parser).
     * Frees the schema when no references to it remain (including those in Parser).
     */
    public function __destruct()
    {
        Library::ffi()->beresta_schema_free($this->pointer);
    }

    /**
     * Дескриптор для вызова Си-функций. / The handle for calling C functions.
     *
     * @internal
     */
    public function handle(): CData
    {
        return $this->pointer;
    }
}
