<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Language;
use Beresta\Native\Calls;
use Beresta\Native\Library;
use Beresta\Native\Strings;
use Beresta\NodeType;
use Beresta\WriteFlags;

/**
 * Трейт узла: тип, имя, текст и сериализация.
 * A node trait: the type, the name, the text and serialization.
 */
trait NodeContent
{
    /**
     * Тип узла; берётся из обёртки без вызова библиотеки.
     * The node type; taken from the wrapper without a library call.
     */
    public function type(): NodeType
    {
        $type = NodeType::tryFrom($this->kind);
        if ($type === null) {
            throw new \UnexpectedValueException(
                Language::message(Message::UnknownNodeType, (string) $this->kind));
        }
        return $type;
    }

    /**
     * Имя: тег элемента, цель PI, имя DOCTYPE; "#text", "#comment" и т. п.
     * The name: the element tag, the PI target, the DOCTYPE name; "#text", "#comment" and so on.
     */
    public function name(): string
    {
        return Strings::take(Library::ffi()->beresta_node_name($this->handle())) ?? '';
    }

    /**
     * Текст узла; у элемента и документа — весь текст потомков.
     * The node text; for an element and a document — all text of the descendants.
     */
    public function text(): string
    {
        return Strings::take(Library::ffi()->beresta_node_text($this->handle())) ?? '';
    }

    /**
     * Замена текста: у элемента все дети заменяются одним текстовым узлом.
     * Replaces the text: for an element all children are replaced with one text node.
     */
    public function setText(string $text): static
    {
        Calls::check(Library::ffi()->beresta_node_set_text($this->handle(), $text));
        return $this;
    }

    /**
     * XML узла и потомков в UTF-8; $flags — маска WriteFlags.
     * The XML of the node and its descendants in UTF-8; $flags is a WriteFlags mask.
     */
    public function toXml(int $flags = WriteFlags::DEFAULT): string
    {
        return Strings::take(Library::ffi()->beresta_node_to_string($this->handle(), $flags)) ?? '';
    }
}
