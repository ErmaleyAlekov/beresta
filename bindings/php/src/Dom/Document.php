<?php

declare(strict_types=1);

namespace Beresta;

use Beresta\Internal\DocumentContent;
use Beresta\Internal\DocumentInternals;
use Beresta\Internal\WrapperRegistry;
use Beresta\Native\Calls;
use Beresta\Native\Handles;
use Beresta\Native\Library;
use FFI\CData;

/**
 * Документ XML — владелец всего дерева узлов; сам тоже узел (к нему
 * добавляют детей, по нему ищут XPath).
 *
 *     $order = Document::create('order');
 *     $order->root()->appendElement('item')->setAttribute('qty', '2');
 *     $order->save('order.xml', WriteFlags::PRETTY);
 *     $copy = clone $order;                // глубокая копия / a deep copy
 *
 * Нативная память освобождается детерминированно: явным close() или, если
 * про это забыли, деструктором, когда на документ и его узлы не останется
 * ссылок (подсчёт ссылок PHP срабатывает сразу, без сборщика мусора).
 * Повторный close() безопасен.
 *
 * An XML document — the owner of the whole node tree; itself a node too (children
 * are appended to it, XPath searches through it).
 *
 * The native memory is freed deterministically: by an explicit close() or, if that
 * was forgotten, by the destructor when no references to the document and its
 * nodes remain (PHP reference counting fires at once, without the garbage
 * collector). A repeated close() is safe.
 */
final class Document extends Node
{
    use DocumentContent;
    use DocumentInternals;

    /**
     * true после close(): память документа уже освобождена.
     * true after close(): the document memory is already freed.
     */
    private bool $closed = false;

    /**
     * Единственные обёртки узлов документа: адрес => обёртка (см. WrapperRegistry).
     * The only wrappers of the document nodes: address => wrapper (see WrapperRegistry).
     */
    private WrapperRegistry $wrappers;

    /**
     * Документ по дескриптору от библиотеки (Parser, create, clone).
     * A document from a library handle (Parser, create, clone).
     *
     * @internal
     */
    public function __construct(CData $pointer)
    {
        parent::__construct($pointer, null, NodeType::Document->value);
        $this->wrappers = new WrapperRegistry();
    }

    /**
     * Новый документ; $rootName === null — без корневого элемента.
     * A new document; $rootName === null means without a root element.
     */
    public static function create(?string $rootName = null): self
    {
        $slot = Handles::slot();
        Calls::check(Library::ffi()->beresta_document_create($rootName, Handles::out($slot)));
        return new self($slot);
    }

    /**
     * Освобождение нативной памяти; узлы документа становятся недоступны.
     * Releases the native memory; the document nodes become unavailable.
     */
    public function close(): void
    {
        if ($this->closed) {
            return;
        }
        $this->closed = true;
        $this->wrappers = new WrapperRegistry();
        Library::ffi()->beresta_document_free($this->pointer);
    }

    /**
     * true — документ закрыт. / true means the document is closed.
     */
    public function isClosed(): bool
    {
        return $this->closed;
    }

    /**
     * Страховка: память освобождается и без явного close().
     * A safety net: the memory is freed even without an explicit close().
     */
    public function __destruct()
    {
        $this->close();
    }
}
