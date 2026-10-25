<?php

declare(strict_types=1);

namespace Beresta\Tests;

use Beresta\BerestaException;
use Beresta\Diagnostic;
use Beresta\Parser;
use Beresta\ParseFlags;
use Beresta\Schema;
use Beresta\Status;
use Beresta\Tests\Support\TestCase;

/**
 * Тесты для класса Schema.
 * Tests for the Schema class.
 */
final class SchemaTest extends TestCase
{
    /**
     * Проверка validate() корректного документа.
     * Validation of a correct document.
     */
    public function testValidateCorrectDocument(): void
    {
        $schema = Schema::fromString('<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"><xs:element name="order"><xs:complexType><xs:sequence><xs:element name="qty" type="xs:int" maxOccurs="unbounded"/></xs:sequence></xs:complexType></xs:element></xs:schema>');
        $document = (new Parser())->parseString('<order><qty>2</qty></order>');
        $problems = $schema->validate($document);
        $this->assertSame([], $problems);
    }

    /**
     * Проверка validate() некорректного документа с отслеживанием местоположений.
     * Validation of an incorrect document with location tracking.
     */
    public function testValidateIncorrectDocumentWithLocations(): void
    {
        $schema = Schema::fromString('<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"><xs:element name="order"><xs:complexType><xs:sequence><xs:element name="qty" type="xs:int" maxOccurs="unbounded"/></xs:sequence></xs:complexType></xs:element></xs:schema>');
        $document = (new Parser(ParseFlags::TRACK_LOCATIONS))->parseString("<order>\n<qty>x</qty>\n<qty>y</qty></order>");
        $problems = $schema->validate($document);
        $this->assertSame(2, count($problems));
        $this->assertSame(2, $problems[0]->line);
        $this->assertTrue($problems[0]->message !== '');
        $this->assertTrue(str_starts_with((string)$problems[0], '2:'));
    }

    /**
     * Проверка validateString() некорректного документа.
     * Validation of an incorrect document using validateString().
     */
    public function testValidateStringIncorrectDocument(): void
    {
        $schema = Schema::fromString('<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"><xs:element name="order"><xs:complexType><xs:sequence><xs:element name="qty" type="xs:int" maxOccurs="unbounded"/></xs:sequence></xs:complexType></xs:element></xs:schema>');
        $problems = $schema->validateString("<order>\n<qty>x</qty>\n<qty>y</qty></order>");
        $this->assertSame(2, count($problems));
    }

    /**
     * Проверка validateString() корректного документа.
     * Validation of a correct document using validateString().
     */
    public function testValidateStringCorrectDocument(): void
    {
        $schema = Schema::fromString('<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"><xs:element name="order"><xs:complexType><xs:sequence><xs:element name="qty" type="xs:int" maxOccurs="unbounded"/></xs:sequence></xs:complexType></xs:element></xs:schema>');
        $problems = $schema->validateString('<order><qty>1</qty></order>');
        $this->assertSame([], $problems);
    }

    /**
     * Проверка validateFile() некорректного документа.
     * Validation of an incorrect document using validateFile().
     */
    public function testValidateFileIncorrectDocument(): void
    {
        $schema = Schema::fromString('<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"><xs:element name="order"><xs:complexType><xs:sequence><xs:element name="qty" type="xs:int" maxOccurs="unbounded"/></xs:sequence></xs:complexType></xs:element></xs:schema>');
        $path = tempnam(sys_get_temp_dir(), 'beresta');
        file_put_contents($path, "<order>\n<qty>x</qty>\n<qty>y</qty></order>");
        $problems = $schema->validateFile($path);
        $this->assertSame(2, count($problems));
        unlink($path);
    }

    /**
     * Проверка Schema::fromString() с некорректной схемой.
     * Schema::fromString() with an incorrect schema.
     */
    public function testSchemaFromStringIncorrectSchema(): void
    {
        $this->assertThrows(BerestaException::class, fn() => Schema::fromString('<xs:schema'));
    }

    /**
     * Проверка разбора с некорректным документом и схемой.
     * Parsing with an incorrect document and schema.
     */
    public function testParsingWithIncorrectDocumentAndSchema(): void
    {
        $schema = Schema::fromString('<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"><xs:element name="order"><xs:complexType><xs:sequence><xs:element name="qty" type="xs:int" maxOccurs="unbounded"/></xs:sequence></xs:complexType></xs:element></xs:schema>');
        $this->assertThrows(BerestaException::class, fn() => (new Parser())->withSchema($schema)->parseString('<order><qty>z</qty></order>'));
    }

    /**
     * Проверка разбора с корректным документом и схемой.
     * Parsing with a correct document and schema.
     */
    public function testParsingWithCorrectDocumentAndSchema(): void
    {
        $schema = Schema::fromString('<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"><xs:element name="order"><xs:complexType><xs:sequence><xs:element name="qty" type="xs:int" maxOccurs="unbounded"/></xs:sequence></xs:complexType></xs:element></xs:schema>');
        $document = (new Parser())->withSchema($schema)->parseString('<order><qty>3</qty></order>');
        $this->assertSame('order', $document->root()->name());
    }

    /**
     * Проверка validateString() некорректного XML.
     * Validation of incorrect XML using validateString().
     */
    public function testValidateStringIncorrectXml(): void
    {
        $schema = Schema::fromString('<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"><xs:element name="order"><xs:complexType><xs:sequence><xs:element name="qty" type="xs:int" maxOccurs="unbounded"/></xs:sequence></xs:complexType></xs:element></xs:schema>');
        $error = $this->assertThrows(BerestaException::class, fn() => $schema->validateString('<order>'));
        $this->assertSame(Status::ParseError, $error->status());
    }
}
