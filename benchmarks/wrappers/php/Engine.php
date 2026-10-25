<?php

declare(strict_types=1);

namespace BerestaBench;

/**
 * Одна сравниваемая библиотека: одинаковые операции для всех (паттерн Strategy).
 *
 * Схема и запрос XPath компилируются один раз в конструкторе, поэтому в
 * замер попадает только сама операция. Результаты walk / xpath /
 * parseValidated — числа для сверки: у всех библиотек они должны совпасть.
 *
 * One compared library: the same operations for all (the Strategy pattern).
 *
 * The schema and the XPath query are compiled once in the constructor, so only the
 * operation itself is measured. The walk / xpath / parseValidated results are
 * numbers for checking: they must match for all libraries.
 */
abstract class Engine
{
    /**
     * @param string $name имя для таблицы / the name for the table
     */
    public function __construct(public readonly string $name)
    {
    }

    /**
     * Разбор в дерево. / Parsing into a tree.
     */
    abstract public function parse(string $xml): object;

    /**
     * Обход всего дерева: число элементов. / Walking the whole tree: the number of elements.
     */
    abstract public function walk(object $document): int;

    /**
     * Запрос XPath: число узлов результата. / An XPath query: the number of result nodes.
     */
    abstract public function xpath(object $document): int;

    /**
     * Разбор с проверкой по XSD: число нарушений; null — библиотека не умеет XSD.
     * Parsing with XSD validation: the number of violations; null means the library cannot do XSD.
     */
    abstract public function parseValidated(string $xml): ?int;
}
