<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Document;
use Beresta\Native\Calls;
use Beresta\Native\Library;
use Beresta\Native\Strings;
use Beresta\Node;
use FFI\CData;

/**
 * Чтение набора узлов XPath в массив PHP и его освобождение.
 *
 * Набор ссылается на узлы документа и действителен, пока документ не
 * изменён, поэтому он читается целиком сразу и тут же освобождается
 * (try/finally) — наружу набор не выходит.
 *
 * Reading an XPath node set into a PHP array and freeing it.
 *
 * The set refers to the document nodes and is valid while the document is not
 * changed, so it is read entirely at once and freed right away (try/finally) —
 * the set never leaves this class.
 *
 * @internal
 */
final class NodeSetReader
{
    /**
     * Узлы-обёртки и строковые значения атрибутов. / Node wrappers and string values of attributes.
     *
     * @return list<Node|string>
     */
    public static function items(CData $set, Document $document): array
    {
        $ffi = Library::ffi();
        try {
            $count = Calls::checkInt($ffi->beresta_nodeset_size($set));
            $result = [];
            for ($index = 0; $index < $count; $index++) {
                $result[] = self::item($set, $index, $document);
            }
            return $result;
        } finally {
            $ffi->beresta_nodeset_free($set);
        }
    }

    /**
     * Строковые значения всех элементов набора. / The string values of all set items.
     *
     * @return list<string>
     */
    public static function values(CData $set): array
    {
        $ffi = Library::ffi();
        try {
            $count = Calls::checkInt($ffi->beresta_nodeset_size($set));
            $result = [];
            for ($index = 0; $index < $count; $index++) {
                $result[] = Strings::take($ffi->beresta_nodeset_value($set, $index)) ?? '';
            }
            return $result;
        } finally {
            $ffi->beresta_nodeset_free($set);
        }
    }

    /**
     * Один элемент: узел или (у атрибута, где узла нет) его значение.
     * One item: a node or (for an attribute, where there is no node) its value.
     */
    private static function item(CData $set, int $index, Document $document): Node|string
    {
        $ffi = Library::ffi();
        $pointer = $ffi->beresta_nodeset_node($set, $index);
        if ($pointer === null) {
            return Strings::take($ffi->beresta_nodeset_value($set, $index)) ?? '';
        }
        return NodeFactory::created($pointer, $document);
    }
}
