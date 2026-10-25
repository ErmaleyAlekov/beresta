<?php

declare(strict_types=1);

namespace Beresta\Tests;

use Beresta\BerestaException;
use Beresta\Document;
use Beresta\Status;
use Beresta\Tests\Support\TestCase;

/**
 * Перенос, копирование и удаление поддерева; единственность обёрток узлов.
 * Moving, copying and removing a subtree; the uniqueness of node wrappers.
 */
final class MoveTest extends TestCase
{
    /**
     * После moveTo объекты узла и потомков, полученные раньше, принадлежат
     * новому документу и переживают закрытие прежнего.
     *
     * After moveTo the node and descendant objects obtained earlier belong to the
     * new document and survive the closing of the former one.
     */
    public function testMovedWrappersFollowTheNode(): void
    {
        $source = Document::create('a');
        $target = Document::create('b');
        $moved = $source->root()->appendElement('x');
        $child = $moved->appendElement('y');
        $moved->moveTo($target->root());
        $source->close();
        $this->assertTrue($moved->document() === $target);
        $this->assertTrue($child->document() === $target);
        $this->assertSame('y', $child->name());
        $this->assertSame('<b><x><y/></x></b>', $target->toXml());
    }

    /**
     * Корень документа переносить нельзя — HIERARCHY_VIOLATION, дерево не меняется.
     * The document root cannot be moved — HIERARCHY_VIOLATION, the tree does not change.
     */
    public function testRootCannotBeMoved(): void
    {
        $source = Document::create('a');
        $target = Document::create('b');
        $error = $this->assertThrows(BerestaException::class, fn() => $source->root()->moveTo($target->root()));
        $this->assertSame(Status::HierarchyViolation, $error->status());
        $this->assertSame('<a/>', $source->toXml());
    }

    /**
     * copyTo — независимая копия в другом документе.
     * copyTo makes an independent copy in another document.
     */
    public function testCopyIsIndependent(): void
    {
        $source = Document::create('a');
        $original = $source->root()->appendElement('x');
        $target = Document::create('b');
        $copy = $original->copyTo($target->root());
        $copy->setText('копия');
        $this->assertTrue($copy->document() === $target);
        $this->assertSame('', $original->text());
        $this->assertSame('<b><x>копия</x></b>', $target->toXml());
    }

    /**
     * Один узел — один объект: разные пути к узлу дают тот же объект.
     * One node — one object: different paths to the node give the same object.
     */
    public function testOneObjectPerNode(): void
    {
        $document = Document::create('a');
        $first = $document->root()->appendElement('x');
        $document->root()->appendElement('y');
        $this->assertTrue($document->root()->firstChild() === $first);
        $this->assertTrue($document->root()->child(1)->previousSibling() === $first);
        $this->assertTrue($first->parent() === $document->root());
        $this->assertTrue($document->root()->parent() === $document);
    }

    /**
     * remove() убирает узел с потомками. / remove() takes the node away with its descendants.
     */
    public function testRemove(): void
    {
        $document = Document::create('a');
        $document->root()->appendElement('x')->appendText('t');
        $document->root()->appendComment('c');
        $document->root()->firstChild()->remove();
        $this->assertSame(1, count($document->root()));
        $this->assertSame('<a><!--c--></a>', $document->toXml());
    }
}
