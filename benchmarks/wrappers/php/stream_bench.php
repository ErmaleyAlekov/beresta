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

/**
 * Одна проверка XMLReader до конца файла: число нарушений.
 * One XMLReader validation to the end of the file: the violation count.
 */
function readerErrors(string $path, string $xsdPath): int
{
    $previous = libxml_use_internal_errors(true);
    $reader = new \XMLReader();
    $reader->open($path);
    $reader->setSchema($xsdPath);
    while ($reader->read()) {
        // Только чтение: проверку делает сам XMLReader
        // Reading only: XMLReader itself does the validation
    }
    $reader->close();
    $errors = count(libxml_get_errors());
    libxml_clear_errors();
    libxml_use_internal_errors($previous);
    return $errors;
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
    $data = realpath($arguments[1] ?? 'data');
    $path = $data . '/catalog.xml';
    $xsdPath = $data . '/catalog.xsd';
    $xml = (string) file_get_contents($path);
    $schema = Schema::fromString((string) file_get_contents($xsdPath));
    $bytes = strlen($xml);
    echo '| Библиотека / Library | Поток + XSD / Stream + XSD, MB/s | Ошибки XSD / XSD errors |', PHP_EOL, '|---|---|---|', PHP_EOL;
    printf("| Берёста / Beresta %s (validateString) | %s | %d |\n", Beresta::version(),
        Measure::timed([$schema, 'validateString'], $xml, $bytes), count($schema->validateString($xml)));
    printf("| Берёста / Beresta %s (validateFile) | %s | %d |\n", Beresta::version(),
        Measure::timed('BerestaBench\\berestaFile', [$schema, $path], $bytes), berestaFile([$schema, $path]));
    printf("| XMLReader + setSchema (libxml2 %s) | %s | %d |\n", LIBXML_DOTTED_VERSION,
        Measure::timed('BerestaBench\\readerFile', [$xsdPath, $path], $bytes), readerFile([$xsdPath, $path]));
}

main($argv);
