<?php

declare(strict_types=1);

namespace Beresta;

use Beresta\Internal\NodeChildren;
use Beresta\Internal\NodeContent;
use Beresta\Internal\NodeEditing;
use Beresta\Internal\NodeInternals;
use Beresta\Internal\NodeNavigation;
use Beresta\Internal\NodeQueries;
use Beresta\Internal\NodeStructure;
use FFI\CData;

/**
 * Узел дерева документа: текст, комментарий, CDATA, инструкция обработки,
 * DOCTYPE; у элементов и документа есть свои подклассы (Element, Document).
 *
 * Методы разнесены по трейтам по смыслу: содержимое, навигация, дети,
 * добавление, структура, запросы XPath. Это принцип разделения интерфейсов
 * (ISP из SOLID): каждый трейт отвечает за одну обязанность и читается
 * отдельно, а класс Node лишь собирает их вместе.
 *
 * Узел держит ссылку на свой документ: пока жив хотя бы один узел, PHP не
 * освободит документ, а после Document::close() любое обращение к узлу даёт
 * ClosedException вместо падения процесса. Узлы создаёт только библиотека
 * (фабрика NodeFactory): у одного узла дерева всегда один объект PHP.
 *
 *     foreach ($document->root() as $child) {          // IteratorAggregate
 *         echo $child->name(), ': ', $child->text(), PHP_EOL;
 *     }
 *     echo count($document->root());                    // Countable
 *     echo $node;                                       // Stringable — XML узла
 *
 * A node of the document tree: a text, a comment, CDATA, a processing
 * instruction, a DOCTYPE; elements and the document have their own subclasses
 * (Element, Document).
 *
 * The methods are split into traits by meaning: content, navigation, children,
 * appending, structure, XPath queries. This is the interface segregation principle
 * (ISP from SOLID): each trait has one responsibility and reads separately, and
 * the Node class only combines them.
 *
 * A node keeps a reference to its document: while at least one node is alive, PHP
 * does not free the document, and after Document::close() any access to the node
 * gives a ClosedException instead of a process crash. Nodes are created only by the
 * library (the NodeFactory factory): one tree node always has one PHP object.
 *
 * @implements \IteratorAggregate<int, Node>
 */
class Node implements \IteratorAggregate, \Countable, \Stringable
{
    use NodeContent;
    use NodeNavigation;
    use NodeChildren;
    use NodeEditing;
    use NodeStructure;
    use NodeQueries;
    use NodeInternals;

    /**
     * Создаётся только фабрикой NodeFactory и классом Document. У самого
     * документа $document — null: владелец документа он сам (Document::document).
     *
     * Created only by the NodeFactory factory and the Document class. For the
     * document itself $document is null: the document owns itself (Document::document).
     *
     * @internal
     */
    public function __construct(
        protected CData $pointer,
        protected ?Document $document,
        protected int $kind,
    ) {
    }

    /**
     * Дескриптор для вызова Си-функций; проверяет, что документ не закрыт.
     * The handle for calling C functions; checks that the document is not closed.
     *
     * @internal
     */
    public function handle(): CData
    {
        if ($this->document()->isClosed()) {
            throw ClosedException::document();
        }
        return $this->pointer;
    }

    /**
     * Документ, которому принадлежит узел (moveTo меняет его).
     * The document the node belongs to (moveTo changes it).
     */
    public function document(): Document
    {
        return $this->document;
    }

    /**
     * true — оба объекта указывают на один и тот же узел дерева.
     * true means both objects point to the same tree node.
     */
    public function equals(Node $other): bool
    {
        return $this->address() === $other->address();
    }

    /**
     * XML узла и его потомков (компактно). / The XML of the node and its descendants (compact).
     */
    public function __toString(): string
    {
        return $this->toXml();
    }
}
