<?php

declare(strict_types=1);

namespace Beresta\Tests;

use Beresta\Beresta;
use Beresta\BerestaException;
use Beresta\ClosedException;
use Beresta\Document;
use Beresta\LibraryException;
use Beresta\Language;
use Beresta\Internal\Message;
use Beresta\Tests\Support\TestCase;

/**
 * Регрессии ручного ревью обёртки.
 * Regressions from the binding review.
 */
final class ReviewBindingTest extends TestCase
{
    /**
     * Подстановка не изменяет плейсхолдер внутри аргумента.
     * Substitution preserves a placeholder inside an argument.
     */
    public function testMessagePreservesLiteralAndIncludesArgument(): void
    {
        $message = Language::message(Message::InvalidArgument, 'a{1}b', 'X', 'Z');
        $this->assertTrue(str_contains($message, 'a{1}b'));
        $this->assertTrue(str_contains($message, 'X'));
    }

    /**
     * Удаление закрывает обёртки всего поддерева.
     * Removing a subtree closes all its wrappers.
     */
    public function testRemoveElementAndAccessOldGrandchildThrowsClosedException(): void
    {
        $document = Document::create('root');
        $parent = $document->root()->appendElement('parent');
        $child = $parent->appendElement('child');
        $grandchild = $child->appendElement('grandchild');
        $child->remove();
        $this->assertThrows(ClosedException::class, fn() => $child->name());
        $this->assertThrows(ClosedException::class, fn() => $grandchild->name());
        $newElement = $parent->appendElement('new');
        $this->assertTrue($newElement !== $grandchild && $newElement !== $child);
        $this->assertSame('new', $newElement->name());
    }

    /**
     * Предзагрузка библиотеки с невалидным путем: ожидается LibraryException.
     * Preloading the library with an invalid path: expecting LibraryException.
     */
    public function testPreloadInvalidPathThrowsLibraryException(): void
    {
        $invalidPath = "invalid\npath";
        $this->assertThrows(LibraryException::class, fn() => Beresta::preload($invalidPath));
    }

    /**
     * Ошибка удаления сохраняет живые обёртки.
     * A failed removal preserves live wrappers.
     */
    public function testFailedRemovePreservesWrappers(): void
    {
        $document = Document::create('root');
        $root = $document->root();
        $child = $root->appendElement('child');
        $this->assertThrows(BerestaException::class, fn() => $document->remove());
        $this->assertSame($root, $document->root());
        $this->assertSame($child, $root->firstChild());
        $this->assertSame('child', $child->name());
    }
}
