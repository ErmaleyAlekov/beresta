<?php

declare(strict_types=1);

namespace BerestaBench;

/**
 * ext/dom (DOMDocument + DOMXPath) — стандартный DOM PHP поверх libxml2.
 * Проверка по XSD — schemaValidateSource после разбора: так её делает ext/dom.
 *
 * ext/dom (DOMDocument + DOMXPath) is the standard PHP DOM over libxml2.
 * XSD validation is schemaValidateSource after parsing: this is how ext/dom does it.
 */
final class DomEngine extends Engine
{
    private ?\DOMXPath $xpath = null;
    private ?object $xpathDocument = null;
    public function __construct(private readonly string $xsd, private readonly string $query)
    {
        parent::__construct('ext/dom (libxml2 ' . LIBXML_DOTTED_VERSION . ')');
    }

    public function parse(string $xml): object
    {
        $document = new \DOMDocument();
        if (!$document->loadXML($xml)) {
            throw new \RuntimeException(BenchInput::INVALID_XML);
        }
        return $document;
    }

    /**
     * Обход тем же способом, что у «Берёсты»: firstChild / nextSibling.
     * The walk is done the same way as for Beresta: firstChild / nextSibling.
     */
    public function walk(object $document): int
    {
        $pending = [$document];
        $count = 0;
        while ($pending !== []) {
            $node = array_pop($pending);
            if ($node->nodeType === XML_ELEMENT_NODE) {
                $count++;
            }
            $child = $node->firstChild;
            while ($child !== null) {
                $pending[] = $child;
                $child = $child->nextSibling;
            }
        }
        return $count;
    }

    public function xpath(object $document): int
    {
        if ($this->xpath === null || $this->xpathDocument !== $document) {
            $this->xpath = new \DOMXPath($document);
            $this->xpathDocument = $document;
        }
        return $this->xpath->query($this->query)->length;
    }

    /**
     * Разбор, затем проверка; нарушения собираются через libxml_get_errors.
     * Parsing, then validation; the violations are collected through libxml_get_errors.
     */
    public function parseValidated(string $xml): ?int
    {
        $previous = libxml_use_internal_errors(true);
        libxml_clear_errors();
        try {
            $document = $this->parse($xml);
            if ($document->schemaValidateSource($this->xsd)) {
                return 0;
            }
            return max(1, XsdOutcome::libxmlErrors());
        } finally {
            libxml_clear_errors();
            libxml_use_internal_errors($previous);
        }
    }
}
