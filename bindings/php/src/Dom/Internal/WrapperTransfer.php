<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Document;
use Beresta\Native\Handles;
use Beresta\Native\Library;
use Beresta\Node;
use FFI\CData;

/**
 * Передача обёрток перенесённого поддерева новому документу (после moveTo).
 *
 * Узлы уже живут в новом документе, поэтому обход нативного поддерева
 * безопасен. Обход итеративный (явный стек): глубина дерева не ограничена
 * стеком вызовов PHP. Если у прежнего документа нет других обёрток, кроме
 * корня поддерева, обходить потомков незачем.
 *
 * Hands the wrappers of a moved subtree over to the new document (after moveTo).
 *
 * The nodes already live in the new document, so walking the native subtree is
 * safe. The walk is iterative (an explicit stack): the tree depth is not limited by
 * the PHP call stack. If the former document has no wrappers other than the
 * subtree root, there is no need to walk the descendants.
 *
 * @internal
 */
final class WrapperTransfer
{
    /**
     * Обёртки поддерева $moved: из карты $source в карту $target.
     * The wrappers of the $moved subtree: from the $source map to the $target map.
     */
    public static function transfer(Node $moved, Document $source, Document $target): void
    {
        $deep = $source->wrappers()->count() > 1;
        $moved->adopt($target);
        $pending = [$moved->handle()];
        while ($pending !== []) {
            $current = array_pop($pending);
            self::moveWrapper(Handles::address($current), $source, $target);
            if ($deep) {
                self::pushChildren($current, $pending);
            }
        }
    }

    /**
     * Одна запись карты: обёртка (если есть) получает новый документ.
     * One map entry: the wrapper (if any) gets the new document.
     */
    private static function moveWrapper(int $address, Document $source, Document $target): void
    {
        $node = $source->wrappers()->forget($address);
        if ($node !== null) {
            $node->adopt($target);
            $target->wrappers()->remember($address, $node);
        }
    }

    /**
     * Дети узла — в стек обхода. / The node children go onto the walk stack.
     *
     * @param list<CData> $pending
     */
    private static function pushChildren(CData $parent, array &$pending): void
    {
        $ffi = Library::ffi();
        $child = $ffi->beresta_node_first_child($parent);
        while ($child !== null) {
            $pending[] = $child;
            $child = $ffi->beresta_node_next_sibling($child);
        }
    }
}
