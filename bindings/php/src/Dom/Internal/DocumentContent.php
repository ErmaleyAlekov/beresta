<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\ClosedException;
use Beresta\Element;
use Beresta\Native\Calls;
use Beresta\Native\Handles;
use Beresta\Native\Library;
use Beresta\WriteFlags;

/**
 * Трейт документа: корень, поиск по ID, запись в файл и копирование (clone).
 * A document trait: the root, the lookup by ID, writing to a file and copying (clone).
 */
trait DocumentContent
{
    /**
     * Корневой элемент или null, если его нет. / The root element or null if there is none.
     */
    public function root(): ?Element
    {
        $node = NodeFactory::wrap(Library::ffi()->beresta_document_root($this->handle()), $this);
        if ($node instanceof Element) {
            return $node;
        }
        return null;
    }

    /**
     * Элемент с атрибутом типа ID (по DTD) или с атрибутом xml:id.
     * An element with an ID-typed attribute (by DTD) or with an xml:id attribute.
     */
    public function elementById(string $id): ?Element
    {
        $node = NodeFactory::wrap(Library::ffi()->beresta_document_element_by_id($this->handle(), $id), $this);
        if ($node instanceof Element) {
            return $node;
        }
        return null;
    }

    /**
     * Запись в файл; $flags — маска WriteFlags, $encoding === null — UTF-8.
     * Writes to a file; $flags is a WriteFlags mask, $encoding === null means UTF-8.
     */
    public function save(string $path, int $flags = WriteFlags::DEFAULT, ?string $encoding = null): void
    {
        Calls::check(Library::ffi()->beresta_document_save($this->handle(), $path, $flags, $encoding));
    }

    /**
     * clone $document — глубокая копия: новый независимый документ. Пока
     * копия не готова, она помечена закрытой: если копирование сорвётся,
     * деструктор не освободит чужой (исходный) дескриптор.
     *
     * clone $document is a deep copy: a new independent document. Until the copy
     * is ready it is marked closed: if copying fails, the destructor does not free
     * someone else's (the original) handle.
     */
    public function __clone()
    {
        // Свойства пока скопированы с оригинала: closed и pointer — его
        // The properties are still copied from the original: closed and pointer are its own
        if ($this->closed) {
            throw ClosedException::document();
        }
        $this->closed = true;
        $slot = Handles::slot();
        Calls::check(Library::ffi()->beresta_document_clone($this->pointer, Handles::out($slot)));
        $this->pointer = $slot;
        $this->address = null;
        $this->wrappers = new WrapperRegistry();
        $this->closed = false;
    }
}
