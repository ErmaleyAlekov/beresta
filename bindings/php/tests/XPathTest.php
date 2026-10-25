<?php

declare(strict_types=1);

namespace Beresta\Tests;

use Beresta\BerestaException;
use Beresta\Document;
use Beresta\Parser;
use Beresta\XPath;
use Beresta\Tests\Support\TestCase;

/**
 * Тесты для класса XPath.
 * Tests for the XPath class.
 */
final class XPathTest extends TestCase
{
    /**
     * Выбор всех элементов item и проверка текста второго.
     * Selecting all item elements and checking the text of the second.
     */
    public function testSelectItems(): void
    {
        $document = (new Parser())->parseString('<shop><item code="A1">Чай</item><item code="B2">Кофе</item></shop>');
        $xpath = new XPath('//item');
        $items = $xpath->select($document->root());
        $this->assertSame(2, count($items));
        $this->assertTrue($items[0] instanceof \Beresta\Element);
        $this->assertTrue($items[1] instanceof \Beresta\Element);
        $this->assertSame('Кофе', $items[1]->text());
    }

    /**
     * Выбор всех атрибутов code и проверка их значений.
     * Selecting all code attributes and checking their values.
     */
    public function testSelectAttributes(): void
    {
        $document = (new Parser())->parseString('<shop><item code="A1">Чай</item><item code="B2">Кофе</item></shop>');
        $xpath = new XPath('//item/@code');
        $codes = $xpath->selectValues($document->root());
        $this->assertSame(['A1', 'B2'], $codes);
        // select() отдаёт атрибуты строками: в DOM «Берёсты» они не узлы
        // select() returns attributes as strings: they are not nodes in the Beresta DOM
        $this->assertSame(['A1', 'B2'], $xpath->select($document));
    }

    /**
     * Выбор значений элементов item и проверка их суммы и количества.
     * Selecting values of item elements and checking their sum and count.
     */
    public function testSelectValuesAndNumber(): void
    {
        $document = (new Parser())->parseString('<shop><item code="A1" price="10">Чай</item><item code="B2" price="5.5">Кофе</item></shop>');
        $xpathValues = new XPath('//item');
        $values = $xpathValues->selectValues($document->root());
        $this->assertSame(['Чай', 'Кофе'], $values);

        $xpathNumber = new XPath('sum(//item/@price)');
        $total = $xpathNumber->number($document->root());
        $this->assertSame(15.5, $total);

        $xpathBoolean = new XPath('count(//item) = 2');
        $count = $xpathBoolean->boolean($document->root());
        $this->assertTrue($count);

        $xpathString = new XPath('//item[2]');
        $secondItem = $xpathString->string($document->root());
        $this->assertSame('Кофе', $secondItem);
    }

    /**
     * Использование переменных в XPath выражении.
     * Using variables in XPath expression.
     */
    public function testVariables(): void
    {
        $document = (new Parser())->parseString('<shop><item code="A1">Чай</item><item code="B2">Кофе</item></shop>');
        $xpath = new XPath('//item[@code = $c]', [], ['c' => 'B2']);
        $this->assertSame('Кофе', $xpath->string($document->root()));

        $xpath->setVariable('c', 'A1');
        $this->assertSame('Чай', $xpath->string($document->root()));

        $xpathNumber = new XPath('//item[$n]', [], ['n' => 2]);
        $this->assertSame('Кофе', $xpathNumber->string($document->root()));
    }

    /**
     * Использование префиксов в XPath выражении.
     * Using prefixes in XPath expression.
     */
    public function testPrefixes(): void
    {
        $document = (new Parser())->parseString('<s:shop xmlns:s="urn:shop"><s:item>x</s:item></s:shop>');
        $xpath = new XPath('//s:item', ['s' => 'urn:shop']);
        $items = $xpath->select($document->root());
        $this->assertSame(1, count($items));
        $this->assertTrue($items[0] instanceof \Beresta\Element);
        $this->assertSame('item', $items[0]->localName());
        $this->assertSame('urn:shop', $items[0]->namespaceUri());
    }

    /**
     * Проверка равенства узлов и их идентичности.
     * Checking node equality and identity.
     */
    public function testNodeEquality(): void
    {
        $document = (new Parser())->parseString('<shop><item code="A1">Чай</item></shop>');
        $xpath = new XPath('//item[@code="A1"]');
        $item = $xpath->select($document->root())[0];
        $firstChild = $document->root()->firstChild();
        $this->assertTrue($item->equals($firstChild));
        $this->assertTrue($item === $firstChild);
        // Короткий запрос от узла (Node::xpath) отдаёт тот же объект
        // A short query from a node (Node::xpath) returns the same object
        $this->assertTrue($document->xpath('//item[@code=$c]', [], ['c' => 'A1'])[0] === $firstChild);
    }

    /**
     * Проверка выброса исключения при неверном выражении XPath.
     * Checking exception thrown for invalid XPath expression.
     */
    public function testInvalidXPathThrows(): void
    {
        $error = $this->assertThrows(BerestaException::class, fn() => new XPath('//['));
        $this->assertSame(\Beresta\Status::XPathError, $error->status());
    }
}
