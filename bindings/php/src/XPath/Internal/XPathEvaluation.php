<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Native\Calls;
use Beresta\Native\Handles;
use Beresta\Native\Library;
use Beresta\Native\Strings;
use Beresta\Node;

/**
 * Трейт XPath: вычисление выражения от узла-контекста.
 * An XPath trait: evaluating the expression from a context node.
 */
trait XPathEvaluation
{
    /**
     * Узлы в порядке документа. Атрибуты в DOM «Берёсты» — не узлы, поэтому
     * вместо них в списке их строковые значения.
     *
     * Nodes in document order. Attributes in the Beresta DOM are not nodes, so the
     * list contains their string values instead.
     *
     * @return list<Node|string>
     */
    public function select(Node $context): array
    {
        $slot = Handles::slot();
        Calls::check(Library::ffi()->beresta_xpath_select($this->handle(), $context->handle(), Handles::out($slot)));
        return NodeSetReader::items($slot, $context->document());
    }

    /**
     * Только строковые значения результата — без объектов-узлов, быстрее всего.
     * Only the string values of the result — without node objects, the fastest way.
     *
     * @return list<string>
     */
    public function selectValues(Node $context): array
    {
        $slot = Handles::slot();
        Calls::check(Library::ffi()->beresta_xpath_select($this->handle(), $context->handle(), Handles::out($slot)));
        return NodeSetReader::values($slot);
    }

    /**
     * Результат как строка (функция string() XPath). / The result as a string (the XPath string() function).
     */
    public function string(Node $context): string
    {
        return Strings::take(Library::ffi()->beresta_xpath_string($this->handle(), $context->handle())) ?? '';
    }

    /**
     * Результат как число (функция number() XPath). / The result as a number (the XPath number() function).
     */
    public function number(Node $context): float
    {
        $value = Handles::number();
        Calls::check(Library::ffi()->beresta_xpath_number($this->handle(), $context->handle(), \FFI::addr($value)));
        return (float) $value->cdata;
    }

    /**
     * Результат как логическое значение (функция boolean() XPath).
     * The result as a boolean (the XPath boolean() function).
     */
    public function boolean(Node $context): bool
    {
        return Calls::checkFlag(Library::ffi()->beresta_xpath_boolean($this->handle(), $context->handle()));
    }
}
