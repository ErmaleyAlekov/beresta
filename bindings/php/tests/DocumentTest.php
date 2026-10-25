<?php

declare(strict_types=1);

namespace Beresta\Tests;

use Beresta\BerestaException;
use Beresta\ClosedException;
use Beresta\Document;
use Beresta\Parser;
use Beresta\Tests\Support\TestCase;

/**
 * Тесты для класса Document.
 * Tests for the Document class.
 */
final class DocumentTest extends TestCase
{
    /**
     * Создание документа, добавление элемента, установка атрибута и текста, проверка toXml().
     * Creating a document, adding an element, setting an attribute and text, checking toXml().
     */
    public function testDocumentCreationAndToXml(): void
    {
        $document = Document::create('order');
        $item = $document->root()->appendElement('item');
        $item->setAttribute('qty', '2');
        $item->appendText('Мёд & орехи');
        $this->assertSame('<order><item qty="2">Мёд &amp; орехи</item></order>', $document->toXml());
    }

    /**
     * Проверка метода attributes() на элементе с атрибутами.
     * Checking the attributes() method on an element with attributes.
     */
    public function testAttributesMethod(): void
    {
        $document = Document::create('root');
        $element = $document->root()->appendElement('item');
        $element->setAttribute('a', '1');
        $element->setAttribute('b', '2');
        $this->assertSame(['a' => '1', 'b' => '2'], $element->attributes());
    }

    /**
     * Проверка ArrayAccess для элемента: установка, проверка наличия, удаление атрибута.
     * Checking ArrayAccess for an element: setting, checking existence, removing an attribute.
     */
    public function testArrayAccess(): void
    {
        $document = Document::create('root');
        $element = $document->root()->appendElement('item');
        $element['code'] = 'X';
        $this->assertTrue(isset($element['code']));
        unset($element['code']);
        $this->assertTrue(!isset($element['code']));
        $this->assertTrue(!$element->hasAttribute('code'));
    }

    /**
     * Проверка клонирования документа: изменение копии не влияет на оригинальный документ.
     * Checking document cloning: changes to the copy do not affect the original document.
     */
    public function testDocumentCloning(): void
    {
        $document = Document::create('root');
        $document->root()->appendText('Original');
        $copy = clone $document;
        $copy->root()->appendText(' Copy');
        $this->assertSame('Original', $document->root()->text());
        $this->assertSame('Original Copy', $copy->root()->text());
    }

    /**
     * Проверка метода close() и isClosed(): после закрытия isClosed() возвращает true, root() бросает исключение.
     * Checking the close() and isClosed() methods: after closing, isClosed() returns true, root() throws an exception.
     */
    public function testCloseMethod(): void
    {
        $document = Document::create('root');
        $document->close();
        $this->assertTrue($document->isClosed());
        $this->assertThrows(ClosedException::class, fn() => $document->root());
    }

    /**
     * Документ освобождается подсчётом ссылок сразу после unset, без сборщика
     * циклов, а живой узел удерживает свой документ.
     *
     * A document is freed by reference counting right after unset, without the cycle
     * collector, and a live node keeps its document alive.
     */
    public function testDocumentIsFreedByReferenceCounting(): void
    {
        gc_disable();
        try {
            $document = (new Parser())->parseString('<a><b>x</b></a>');
            $child = $document->root()->firstChild();
            $reference = \WeakReference::create($document);
            unset($document);
            $this->assertSame('x', $child->text());
            unset($child);
            $this->assertNull($reference->get());
        } finally {
            gc_enable();
        }
    }

    /**
     * Проверка метода save(): сохранение в файл и чтение из файла дает одинаковый результат.
     * Checking the save() method: saving to a file and reading from a file gives the same result.
     */
    public function testSaveMethod(): void
    {
        $document = Document::create('root');
        $document->root()->appendText('Test');
        $path = tempnam(sys_get_temp_dir(), 'beresta');
        $document->save($path);
        $parsed = (new Parser())->parseFile($path);
        $this->assertSame('root', $parsed->root()->name());
        unlink($path);
    }

    /**
     * Проверка Countable и IteratorAggregate для корня документа: количество детей и перебор.
     * Checking Countable and IteratorAggregate for the document root: number of children and iteration.
     */
    public function testCountableAndIteratorAggregate(): void
    {
        $document = Document::create('a');
        $root = $document->root();
        $root->appendElement('b');
        $root->appendElement('c');
        $root->appendElement('d');
        $this->assertSame(3, count($root));
        $names = [];
        foreach ($root as $child) {
            $names[] = $child->name();
        }
        $this->assertSame(['b', 'c', 'd'], $names);
    }

    /**
     * Проверка метода elementById(): поиск элемента по атрибуту xml:id.
     * Checking the elementById() method: finding an element by the xml:id attribute.
     */
    public function testElementByIdMethod(): void
    {
        $document = Document::create('a');
        $element = $document->root()->appendElement('b');
        $element->setAttribute('xml:id', 'k');
        $this->assertSame('b', $document->elementById('k')->name());
        $this->assertNull($document->elementById('нет'));
    }
}
