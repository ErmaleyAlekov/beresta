<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Native\Calls;
use Beresta\Native\Library;
use Beresta\Node;

/**
 * Трейт узла: дети по индексу, их число и перебор в foreach.
 * A node trait: the children by index, their number and iteration in foreach.
 */
trait NodeChildren
{
    /**
     * Число детей (Countable: count($node)); у листовых узлов — 0.
     * The number of children (Countable: count($node)); 0 for leaf nodes.
     */
    public function count(): int
    {
        return Calls::checkInt(Library::ffi()->beresta_node_child_count($this->handle()));
    }

    /**
     * Ребёнок по индексу с нуля; вне диапазона — BerestaException INDEX_OUT_OF_RANGE.
     * A child by a zero-based index; out of range is a BerestaException INDEX_OUT_OF_RANGE.
     */
    public function child(int $index): ?Node
    {
        return NodeFactory::wrap(Library::ffi()->beresta_node_child($this->handle(), $index), $this->document());
    }

    /**
     * Все дети списком. / All children as a list.
     *
     * @return list<Node>
     */
    public function children(): array
    {
        return iterator_to_array($this->getIterator(), false);
    }

    /**
     * Перебор детей от первого к последнему (IteratorAggregate). Следующий
     * сосед берётся на каждом шаге, поэтому удалять текущий узел в цикле нельзя.
     *
     * Iterates the children from the first to the last (IteratorAggregate). The next
     * sibling is taken at every step, so the current node must not be removed in the loop.
     *
     * @return \Generator<int, Node>
     */
    public function getIterator(): \Generator
    {
        $child = $this->firstChild();
        while ($child !== null) {
            yield $child;
            $child = $child->nextSibling();
        }
    }
}
