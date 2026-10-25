<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Native\Library;
use Beresta\Node;

/**
 * Трейт узла: переходы к родителю и соседям. null — такого узла нет.
 * A node trait: moving to the parent and the siblings. null means there is no such node.
 */
trait NodeNavigation
{
    /**
     * Родитель; у корневого элемента — сам документ, у документа — null.
     * The parent; for the root element it is the document, for the document it is null.
     */
    public function parent(): ?Node
    {
        return NodeFactory::wrap(Library::ffi()->beresta_node_parent($this->handle()), $this->document());
    }

    /**
     * Первый ребёнок. / The first child.
     */
    public function firstChild(): ?Node
    {
        return NodeFactory::wrap(Library::ffi()->beresta_node_first_child($this->handle()), $this->document());
    }

    /**
     * Последний ребёнок. / The last child.
     */
    public function lastChild(): ?Node
    {
        return NodeFactory::wrap(Library::ffi()->beresta_node_last_child($this->handle()), $this->document());
    }

    /**
     * Следующий сосед. / The next sibling.
     */
    public function nextSibling(): ?Node
    {
        return NodeFactory::wrap(Library::ffi()->beresta_node_next_sibling($this->handle()), $this->document());
    }

    /**
     * Предыдущий сосед. / The previous sibling.
     */
    public function previousSibling(): ?Node
    {
        return NodeFactory::wrap(Library::ffi()->beresta_node_previous_sibling($this->handle()), $this->document());
    }
}
