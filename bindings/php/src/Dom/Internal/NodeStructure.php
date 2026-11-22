<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Native\Calls;
use Beresta\Native\Handles;
use Beresta\Native\Library;
use Beresta\Node;

/**
 * Трейт узла: удаление, копирование и перенос поддерева.
 * A node trait: removing, copying and moving a subtree.
 */
trait NodeStructure
{
    /**
     * Удаление узла с потомками. После вызова этот объект и объекты потомков
     * недействительны — обращаться к ним нельзя.
     *
     * Removes the node with its descendants. After the call this object and the
     * objects of its descendants are invalid — they must not be used.
     */
    public function remove(): void
    {
        $registry = $this->document()->wrappers();
        $pending = [$this->handle()];
        $affected = [];
        while ($pending !== []) {
            $current = array_pop($pending);
            $address = Handles::address($current);
            $wrapper = $registry->find($address);
            if ($wrapper !== null) {
                $affected[$address] = $wrapper;
            }
            WrapperTransfer::pushChildren($current, $pending);
        }
        Calls::check(Library::ffi()->beresta_node_remove($this->handle()));
        $this->removed = true;
        foreach ($affected as $address => $wrapper) {
            $wrapper->removed = true;
            $registry->forget($address);
        }
    }

    /**
     * Глубокая копия в конец детей $parent (можно в другой документ).
     * A deep copy appended to the children of $parent (another document is allowed).
     */
    public function copyTo(Node $parent): Node
    {
        $slot = Handles::slot();
        Calls::check(Library::ffi()->beresta_node_copy_to($this->handle(), $parent->handle(), Handles::out($slot)));
        return NodeFactory::created($slot, $parent->document());
    }

    /**
     * Перенос без копирования в конец детей $parent, в том числе в другой
     * документ. Этот объект остаётся тем же узлом, но его документ теперь —
     * $parent->document(); объекты потомков, полученные раньше, тоже переходят
     * к новому документу. Корень документа и сам документ переносить нельзя
     * (BerestaException HIERARCHY_VIOLATION / INVALID_ARGUMENT).
     *
     * Moves the node without copying to the end of the children of $parent,
     * including into another document. This object stays the same node, but its
     * document is now $parent->document(); the objects of descendants obtained
     * earlier move to the new document too. The document root and the document
     * itself cannot be moved (BerestaException HIERARCHY_VIOLATION / INVALID_ARGUMENT).
     */
    public function moveTo(Node $parent): static
    {
        $source = $this->document();
        $target = $parent->document();
        Calls::check(Library::ffi()->beresta_node_move_to($this->handle(), $parent->handle()));
        if ($source !== $target) {
            WrapperTransfer::transfer($this, $source, $target);
        }
        return $this;
    }
}
