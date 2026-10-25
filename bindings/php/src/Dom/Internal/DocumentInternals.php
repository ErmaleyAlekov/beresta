<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Document;

/**
 * Служебная часть документа: доступ к карте обёрток его узлов и сам
 * документ как владелец.
 *
 * The internal part of a document: access to the wrapper map of its nodes and
 * the document itself as the owner.
 *
 * @internal
 */
trait DocumentInternals
{
    /**
     * Карта «адрес узла => обёртка» (паттерн Identity Map).
     * The "node address => wrapper" map (the Identity Map pattern).
     *
     * @internal
     */
    public function wrappers(): WrapperRegistry
    {
        return $this->wrappers;
    }

    /**
     * Владелец документа — он сам. Ссылка на себя не хранится в свойстве:
     * цикл «документ -> документ» не дал бы подсчёту ссылок освободить его
     * сразу после unset, и память ждала бы сборщика циклов.
     *
     * The owner of a document is itself. The reference to itself is not kept in a
     * property: a "document -> document" cycle would keep reference counting from
     * freeing it right after unset, and the memory would wait for the cycle collector.
     */
    public function document(): Document
    {
        return $this;
    }
}
