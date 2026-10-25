<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\BerestaException;
use Beresta\Document;
use Beresta\Element;
use Beresta\Language;
use Beresta\Native\Calls;
use Beresta\Native\Handles;
use Beresta\Native\Library;
use Beresta\Node;
use Beresta\NodeType;
use FFI\CData;

/**
 * Фабрика объектов-обёрток для дескрипторов узлов (паттерн Factory Method).
 *
 * Си-интерфейс отдаёт «голый» указатель; по его типу выбирается класс PHP:
 * Element для элементов, сам документ для дескриптора документа, Node для
 * остальных узлов. Уже выданная обёртка берётся из карты документа
 * (WrapperRegistry). Запись об удалённом узле может пережить его — адрес
 * достанется новому узлу, — поэтому сверяется и запомненный тип узла.
 *
 * The factory of wrapper objects for node handles (the Factory Method pattern).
 *
 * The C interface returns a "bare" pointer; the PHP class is chosen by its type:
 * Element for elements, the document itself for the document handle, Node for
 * other nodes. A wrapper handed out before is taken from the document map
 * (WrapperRegistry). The entry of a removed node may outlive it — the address goes
 * to a new node — so the remembered node type is compared too.
 *
 * @internal
 */
final class NodeFactory
{
    /**
     * Обёртка узла документа $document (NULL без ошибки — null).
     * A wrapper for a node of the $document document (NULL without an error is null).
     */
    public static function wrap(?CData $pointer, Document $document): ?Node
    {
        if (Calls::checkHandle($pointer) === null) {
            return null;
        }
        $address = Handles::address($pointer);
        if ($address === $document->address()) {
            return $document;
        }
        $kind = Calls::checkInt(Library::ffi()->beresta_node_type($pointer));
        $known = $document->wrappers()->find($address);
        if ($known !== null && $known->kindCode() === $kind) {
            return $known;
        }
        $node = self::create($pointer, $document, $kind);
        $document->wrappers()->remember($address, $node);
        return $node;
    }

    /**
     * Обёртка только что созданного узла: NULL здесь всегда ошибка.
     * A wrapper for a just created node: NULL here is always an error.
     */
    public static function created(?CData $pointer, Document $document): Node
    {
        $node = self::wrap($pointer, $document);
        if ($node === null) {
            throw BerestaException::fromLastError();
        }
        return $node;
    }

    /**
     * То же для элемента (appendElement). / The same for an element (appendElement).
     */
    public static function element(?CData $pointer, Document $document): Element
    {
        $node = self::created($pointer, $document);
        if (!$node instanceof Element) {
            throw new \UnexpectedValueException(
                Language::message(Message::UnknownNodeType, (string) $node->kindCode()));
        }
        return $node;
    }

    /**
     * Класс обёртки по типу узла. / The wrapper class by the node type.
     */
    private static function create(CData $pointer, Document $document, int $kind): Node
    {
        if ($kind === NodeType::Element->value) {
            return new Element($pointer, $document, $kind);
        }
        return new Node($pointer, $document, $kind);
    }
}
