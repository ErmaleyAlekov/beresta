<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Document;
use Beresta\Native\Handles;

/**
 * Служебная часть узла: адрес, код типа и смена документа-владельца.
 * Нужна фабрике узлов и переносу поддерева (moveTo), а не пользователю.
 *
 * The internal part of a node: the address, the type code and the change of the
 * owner document. Needed by the node factory and the subtree move (moveTo), not
 * by the user.
 *
 * @internal
 */
trait NodeInternals
{
    /**
     * Адрес узла, вычисленный при первом обращении (адрес узла не меняется).
     * The node address computed on the first access (a node address never changes).
     */
    protected ?int $address = null;

    /**
     * Адрес узла в памяти — ключ карты обёрток документа.
     * The node address in memory — the key of the document wrapper map.
     *
     * @internal
     */
    public function address(): int
    {
        if ($this->address === null) {
            $this->address = Handles::address($this->pointer);
        }
        return $this->address;
    }

    /**
     * Код типа узла, запомненный при создании обёртки (тип узла не меняется).
     * The node type code remembered when the wrapper was created (a node type never changes).
     *
     * @internal
     */
    public function kindCode(): int
    {
        return $this->kind;
    }

    /**
     * Новый документ-владелец после beresta_node_move_to.
     * The new owner document after beresta_node_move_to.
     *
     * @internal
     */
    public function adopt(Document $document): void
    {
        $this->document = $document;
    }
}
