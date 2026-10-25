<?php

declare(strict_types=1);

namespace BerestaBench;

use Beresta\Beresta;
use Beresta\Element;
use Beresta\Parser;
use Beresta\Schema;
use Beresta\XPath;

/**
 * «Берёста» через обёртку FFI: каждый шаг обхода — вызов FFI и объект PHP.
 * Beresta through the FFI wrapper: every walk step is an FFI call and a PHP object.
 */
final class BerestaEngine extends Engine
{
    private readonly Parser $parser;
    private readonly Parser $validating;
    private readonly XPath $query;

    public function __construct(string $xsd, string $query)
    {
        parent::__construct('Берёста / Beresta ' . Beresta::version() . ' (FFI)');
        $this->parser = new Parser();
        $this->validating = $this->parser->withSchema(Schema::fromString($xsd));
        $this->query = new XPath($query);
    }

    public function parse(string $xml): object
    {
        return $this->parser->parseString($xml);
    }

    /**
     * Обход явным стеком: firstChild / nextSibling у каждого узла.
     * A walk with an explicit stack: firstChild / nextSibling of every node.
     */
    public function walk(object $document): int
    {
        $pending = [$document];
        $count = 0;
        while ($pending !== []) {
            $node = array_pop($pending);
            if ($node instanceof Element) {
                $count++;
            }
            $child = $node->firstChild();
            while ($child !== null) {
                $pending[] = $child;
                $child = $child->nextSibling();
            }
        }
        return $count;
    }

    public function xpath(object $document): int
    {
        return count($this->query->select($document));
    }

    /**
     * Проверка идёт во время разбора; нарушение — исключение.
     * Validation runs during parsing; a violation is an exception.
     */
    public function parseValidated(string $xml): ?int
    {
        $this->validating->parseString($xml);
        return 0;
    }
}
