<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Node;

/**
 * Карта «адрес узла => обёртка» одного документа (паттерн Identity Map).
 *
 * Зачем: у одного узла дерева должен быть один объект PHP. Иначе при
 * переносе узла в другой документ (moveTo) объекты, полученные раньше,
 * продолжали бы считать владельцем прежний документ: после закрытия
 * нового документа они обратились бы к освобождённой памяти.
 *
 * Ссылки слабые (WeakReference): карта не удерживает обёртки, ненужные
 * пользователю. Мёртвые записи вычищаются, когда карта вырастает вдвое
 * с прошлой чистки, — так затраты на чистку остаются O(1) на запись.
 *
 * The "node address => wrapper" map of one document (the Identity Map pattern).
 *
 * Why: one tree node must have one PHP object. Otherwise, when a node is moved into
 * another document (moveTo), the objects obtained earlier would keep the former
 * document as the owner: after the new document is closed they would access
 * freed memory.
 *
 * The references are weak (WeakReference): the map does not hold wrappers the
 * user no longer needs. Dead entries are swept when the map doubles since the
 * last sweep — so the sweeping cost stays O(1) per entry.
 *
 * @internal
 */
final class WrapperRegistry
{
    /**
     * Размер карты, ниже которого чистка не нужна. / The map size below which no sweep is needed.
     */
    private const MIN_SWEEP = 1024;

    /** @var array<int, \WeakReference<Node>> */
    private array $items = [];

    /**
     * Размер, при котором выполнится следующая чистка. / The size at which the next sweep runs.
     */
    private int $sweepAt = self::MIN_SWEEP;

    /**
     * Живая обёртка узла по адресу или null. / A live wrapper of the node by address or null.
     */
    public function find(int $address): ?Node
    {
        return ($this->items[$address] ?? null)?->get();
    }

    /**
     * Запоминание обёртки (запись с тем же адресом заменяется).
     * Remembers a wrapper (an entry with the same address is replaced).
     */
    public function remember(int $address, Node $node): void
    {
        $this->items[$address] = \WeakReference::create($node);
        if (count($this->items) >= $this->sweepAt) {
            $this->sweep();
        }
    }

    /**
     * Удаление записи; возвращает живую обёртку, если она была.
     * Removes an entry; returns the live wrapper if there was one.
     */
    public function forget(int $address): ?Node
    {
        $node = $this->find($address);
        unset($this->items[$address]);
        return $node;
    }

    /**
     * Число записей (вместе с ещё не вычищенными мёртвыми).
     * The number of entries (including dead ones not yet swept).
     */
    public function count(): int
    {
        return count($this->items);
    }

    /**
     * Чистка мёртвых записей и новый порог — вдвое больше оставшихся.
     * Sweeps dead entries and sets a new threshold — twice the remaining ones.
     */
    private function sweep(): void
    {
        foreach ($this->items as $address => $reference) {
            if ($reference->get() === null) {
                unset($this->items[$address]);
            }
        }
        $this->sweepAt = max(self::MIN_SWEEP, 2 * count($this->items));
    }
}
