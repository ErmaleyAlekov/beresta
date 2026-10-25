<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Element;
use Beresta\Native\Library;
use Beresta\Node;

/**
 * Трейт узла: добавление новых узлов в конец детей (элемента или документа).
 * Каждый метод возвращает созданный узел — дерево удобно строить цепочкой.
 *
 * A node trait: appending new nodes to the end of the children (of an element or
 * a document). Each method returns the created node — convenient for building a
 * tree in a chain.
 */
trait NodeEditing
{
    /**
     * Новый элемент; у пустого документа он становится корнем.
     * A new element; in an empty document it becomes the root.
     */
    public function appendElement(string $name): Element
    {
        return NodeFactory::element(Library::ffi()->beresta_node_append_element($this->handle(), $name), $this->document());
    }

    /**
     * Новый текстовый узел (& и < экранируются при выводе).
     * A new text node (& and < are escaped on output).
     */
    public function appendText(string $text): Node
    {
        return NodeFactory::created(Library::ffi()->beresta_node_append_text($this->handle(), $text), $this->document());
    }

    /**
     * Новый комментарий. / A new comment.
     */
    public function appendComment(string $text): Node
    {
        return NodeFactory::created(Library::ffi()->beresta_node_append_comment($this->handle(), $text), $this->document());
    }

    /**
     * Новая секция CDATA. / A new CDATA section.
     */
    public function appendCdata(string $text): Node
    {
        return NodeFactory::created(Library::ffi()->beresta_node_append_cdata($this->handle(), $text), $this->document());
    }

    /**
     * Инструкция обработки <?target data?>. / A processing instruction <?target data?>.
     */
    public function appendPi(string $target, ?string $data = null): Node
    {
        $pointer = Library::ffi()->beresta_node_append_pi($this->handle(), $target, $data);
        return NodeFactory::created($pointer, $this->document());
    }
}
