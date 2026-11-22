<?php

declare(strict_types=1);

/*
 * Потоковая проверка по XSD без дерева на тех же данных, что и bench.php:
 * «Берёста» (Schema::validateString / validateFile — SAX + XSD внутри ядра)
 * против XMLReader со схемой (setSchema, libxml2) — штатного способа PHP
 * проверить документ без DOM. Медиана из 5 прогонов после прогревочного.
 *     php stream_bench.php data
 *
 * Streaming XSD validation without a tree on the same data as bench.php:
 * Beresta (Schema::validateString / validateFile — SAX + XSD inside the core)
 * against XMLReader with a schema (setSchema, libxml2) — the standard PHP way to
 * validate a document without DOM. The median of 5 runs after a warm-up run.
 *     php stream_bench.php data
 */

namespace BerestaBench;

use Beresta\Beresta;
use Beresta\Schema;

require_once __DIR__ . '/../../../bindings/php/autoload.php';
require_once __DIR__ . '/Engine.php';
require_once __DIR__ . '/Measure.php';
require_once __DIR__ . '/BenchInput.php';
require_once __DIR__ . '/XsdOutcome.php';

/**
 * Одна проверка XMLReader до конца файла: число нарушений.
 * One XMLReader validation to the end of the file: the violation count.
 */
function readerErrors(string $path, string $xsdPath): int
{
    $previous = libxml_use_internal_errors(true);
    libxml_clear_errors();
    try {
        $reader = new \XMLReader();
        if (!$reader->open($path)) {
            throw new \RuntimeException(BenchInput::READ_FAILED . $path);
        }
        try {
            if (!$reader->setSchema($xsdPath)) {
                throw new \RuntimeException(BenchInput::INVALID_XSD . $xsdPath);
            }
            while ($reader->read()) {
                // Проверку делает XMLReader. / XMLReader performs validation.
            }
            if ($reader->isValid()) {
                return 0;
            }
            return max(1, XsdOutcome::libxmlErrors());
        } finally {
            $reader->close();
        }
    } finally {
        libxml_clear_errors();
        libxml_use_internal_errors($previous);
    }
}

/**
 * Проверка «Берёстой» файла и XMLReader — замыкания здесь не нужны:
 * аргумент операции — массив [схема или путь XSD, путь к файлу].
 *
 * Validating a file with Beresta and XMLReader — no closures are needed here:
 * the operation argument is an array [a schema or an XSD path, the file path].
 *
 * @param array{0: Schema, 1: string} $job
 */
function berestaFile(array $job): int
{
    return count($job[0]->validateFile($job[1]));
}

/**
 * @param array{0: string, 1: string} $job
 */
function readerFile(array $job): int
{
    return readerErrors($job[1], $job[0]);
}

/**
 * Таблица Markdown. / A Markdown table.
 *
 * @param list<string> $arguments
 */
function main(array $arguments): void
{
    $data = $arguments[1] ?? 'data';
    $path = $data . '/catalog.xml';
    $xsdPath = $data . '/catalog.xsd';
    $xml = BenchInput::read($path);
    $schema = Schema::fromString(BenchInput::read($xsdPath));
    $bytes = strlen($xml);
    echo '| Библиотека / Library | Поток + XSD / Stream + XSD, MB/s | Ошибки XSD / XSD errors | XSD valid |', PHP_EOL, '|---|---|---|---|', PHP_EOL;
    $stringErrors = count($schema->validateString($xml));
    printf("| Берёста / Beresta %s (validateString) | %s | %d | %s |\n", Beresta::version(),
        Measure::timed([$schema, 'validateString'], $xml, $bytes), $stringErrors, XsdOutcome::validity($stringErrors));
    $fileErrors = berestaFile([$schema, $path]);
    printf("| Берёста / Beresta %s (validateFile) | %s | %d | %s |\n", Beresta::version(),
        Measure::timed('BerestaBench\\berestaFile', [$schema, $path], $bytes), $fileErrors, XsdOutcome::validity($fileErrors));
    $readerErrors = readerFile([$xsdPath, $path]);
    printf("| XMLReader + setSchema (libxml2 %s) | %s | %d | %s |\n", LIBXML_DOTTED_VERSION,
        Measure::timed('BerestaBench\\readerFile', [$xsdPath, $path], $bytes), $readerErrors, XsdOutcome::validity($readerErrors));
}

try {
    main($argv);
} catch (\RuntimeException $failure) {
    fwrite(STDERR, $failure->getMessage() . PHP_EOL);
    exit(2);
}
