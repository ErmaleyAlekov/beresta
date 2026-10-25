<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Node;
use Beresta\XPath;

/**
 * Трейт узла: короткий запрос XPath от этого узла, как у SimpleXML.
 *
 * Выражение компилируется на каждый вызов. Для запроса в цикле выгоднее
 * один раз создать new XPath(...) и вычислять его многократно.
 *
 * A node trait: a short XPath query from this node, as in SimpleXML.
 *
 * The expression is compiled on every call. For a query in a loop it is better
 * to create new XPath(...) once and evaluate it many times.
 */
trait NodeQueries
{
    /**
     * Узлы по выражению; атрибуты — не узлы, вместо них их строковые значения.
     * The nodes by the expression; attributes are not nodes, their string values come instead.
     *
     * @param array<string, string> $namespaces префикс => URI / prefix => URI
     * @param array<string, string|int|float|\Stringable> $variables имя => значение / name => value
     * @return list<Node|string>
     */
    public function xpath(string $expression, array $namespaces = [], array $variables = []): array
    {
        return (new XPath($expression, $namespaces, $variables))->select($this);
    }
}
