<?php

declare(strict_types=1);

namespace BerestaBench;

/**
 * SimpleXML — самый популярный в PHP способ читать XML (тоже libxml2).
 * Проверки по XSD у SimpleXML нет.
 *
 * SimpleXML is the most popular way to read XML in PHP (libxml2 as well).
 * SimpleXML has no XSD validation.
 */
final class SimpleXmlEngine extends Engine
{
    public function __construct(private readonly string $query)
    {
        parent::__construct('SimpleXML (libxml2 ' . LIBXML_DOTTED_VERSION . ')');
    }

    public function parse(string $xml): object
    {
        return simplexml_load_string($xml);
    }

    /**
     * SimpleXML перебирает только элементы: обход через children().
     * SimpleXML iterates only elements: the walk goes through children().
     */
    public function walk(object $document): int
    {
        $pending = [$document];
        $count = 0;
        while ($pending !== []) {
            $element = array_pop($pending);
            $count++;
            foreach ($element->children() as $child) {
                $pending[] = $child;
            }
        }
        return $count;
    }

    public function xpath(object $document): int
    {
        return count($document->xpath($this->query));
    }

    public function parseValidated(string $xml): ?int
    {
        return null;
    }
}
