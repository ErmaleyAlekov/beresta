<?php

declare(strict_types=1);

namespace Beresta;

use Beresta\Native\Calls;
use Beresta\Native\Handles;
use Beresta\Native\Library;

/**
 * Разборщик XML: флаги и необязательная схема для проверки при разборе.
 *
 *     $parser = new Parser(ParseFlags::TRACK_LOCATIONS);
 *     $document = $parser->parseString('<shop><item code="A1">Чай</item></shop>');
 *     $checked = $parser->withSchema(Schema::fromFile('shop.xsd'))->parseFile('shop.xml');
 *
 * Неизменяемый объект (readonly): методы with* возвращают новый разборщик,
 * поэтому один экземпляр можно спокойно хранить в контейнере зависимостей.
 * Строка передаётся в библиотеку как есть: кодировку (UTF-8, UTF-16,
 * windows-1251...) библиотека определит сама по объявлению и BOM.
 *
 * An XML parser: the flags and an optional schema for validation while parsing.
 *
 * An immutable object (readonly): the with* methods return a new parser, so one
 * instance can safely be kept in a dependency container. The string is passed to
 * the library as is: the library detects the encoding (UTF-8, UTF-16,
 * windows-1251...) itself from the declaration and the BOM.
 */
final readonly class Parser
{
    /**
     * @param int $flags маска ParseFlags / a ParseFlags mask
     * @param Schema|null $schema схема для проверки при разборе / a schema to validate against while parsing
     */
    public function __construct(
        private int $flags = ParseFlags::DEFAULT,
        private ?Schema $schema = null,
    ) {
    }

    /**
     * Копия с другими флагами. / A copy with other flags.
     */
    public function withFlags(int $flags): self
    {
        return new self($flags, $this->schema);
    }

    /**
     * Копия с другой схемой (null — без проверки). / A copy with another schema (null means no validation).
     */
    public function withSchema(?Schema $schema): self
    {
        return new self($this->flags, $schema);
    }

    /**
     * Разбор XML из строки (байты в любой поддерживаемой кодировке).
     * Parses XML from a string (bytes in any supported encoding).
     */
    public function parseString(string $xml): Document
    {
        $slot = Handles::slot();
        $status = Library::ffi()->beresta_document_parse(
            $xml, strlen($xml), $this->flags, $this->schema?->handle(), Handles::out($slot));
        Calls::check($status);
        return new Document($slot);
    }

    /**
     * Разбор файла; ParseFlags::FILE_RESOURCES разрешает внешние DTD рядом с ним.
     * Parses a file; ParseFlags::FILE_RESOURCES allows external DTDs next to it.
     */
    public function parseFile(string $path): Document
    {
        $slot = Handles::slot();
        $status = Library::ffi()->beresta_document_parse_file(
            $path, $this->flags, $this->schema?->handle(), Handles::out($slot));
        Calls::check($status);
        return new Document($slot);
    }
}
