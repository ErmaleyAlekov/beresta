<?php

declare(strict_types=1);

namespace Beresta;

use Beresta\Internal\ElementArrayAccess;
use Beresta\Internal\ElementAttributes;
use Beresta\Native\Library;
use Beresta\Native\Strings;

/**
 * Элемент XML: узел с именем, атрибутами и пространством имён.
 *
 * Атрибуты доступны и методами в духе ext/dom, и как элементы массива —
 * как в SimpleXML:
 *
 *     $item->getAttribute('code');      // 'A1' или null / 'A1' or null
 *     $item['code'] = 'B2';             // ArrayAccess: запись / writing
 *     isset($item['price']);            // есть ли атрибут / whether the attribute exists
 *     unset($item['price']);            // удаление / removal
 *
 * An XML element: a node with a name, attributes and a namespace.
 * The attributes are available both through ext/dom-style methods and as array
 * elements — as in SimpleXML (see the example above).
 *
 * @implements \ArrayAccess<string, string>
 */
final class Element extends Node implements \ArrayAccess
{
    use ElementAttributes;
    use ElementArrayAccess;

    /**
     * Имя без префикса: у <s:item> — "item". / The name without a prefix: "item" for <s:item>.
     */
    public function localName(): string
    {
        return Strings::take(Library::ffi()->beresta_element_local_name($this->handle())) ?? '';
    }

    /**
     * URI пространства имён или null, если элемент вне пространств имён.
     * The namespace URI or null if the element is in no namespace.
     */
    public function namespaceUri(): ?string
    {
        return Strings::take(Library::ffi()->beresta_element_namespace_uri($this->handle()));
    }
}
