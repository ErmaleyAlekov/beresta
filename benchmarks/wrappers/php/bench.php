<?php

declare(strict_types=1);

/*
 * Замер PHP-обёртки против ext/dom и SimpleXML на общих данных
 * (benchmarks/wrappers/data, см. make_data.js). Результат — строки Markdown.
 *     php bench.php data                 — таблица скорости
 *     php bench.php data --memory N      — память библиотеки №N (Linux, свой процесс)
 *
 * A measurement of the PHP wrapper against ext/dom and SimpleXML on common data
 * (benchmarks/wrappers/data, see make_data.js). The result is Markdown rows.
 *     php bench.php data                 — the speed table
 *     php bench.php data --memory N      — the memory of library #N (Linux, its own process)
 */

namespace BerestaBench;

require_once __DIR__ . '/../../../bindings/php/autoload.php';
require_once __DIR__ . '/Engine.php';
require_once __DIR__ . '/BerestaEngine.php';
require_once __DIR__ . '/DomEngine.php';
require_once __DIR__ . '/SimpleXmlEngine.php';
require_once __DIR__ . '/Measure.php';

/**
 * Библиотеки в порядке строк таблицы. / The libraries in the table row order.
 *
 * @return list<Engine>
 */
function engines(string $xsd, string $query): array
{
    return [new BerestaEngine($xsd, $query), new DomEngine($xsd, $query), new SimpleXmlEngine($query)];
}

/**
 * Строка таблицы одной библиотеки. / The table row of one library.
 */
function row(Engine $engine, string $xml): string
{
    $document = $engine->parse($xml);
    $errors = $engine->parseValidated($xml);
    $validated = '—';
    if ($errors !== null) {
        $validated = Measure::timed([$engine, 'parseValidated'], $xml, strlen($xml));
    }
    $cells = [$engine->name, Measure::timed([$engine, 'parse'], $xml, strlen($xml)),
        Measure::timed([$engine, 'walk'], $document), Measure::timed([$engine, 'xpath'], $document), $validated,
        (string) $engine->walk($document), (string) $engine->xpath($document), (string) ($errors ?? '—')];
    return '| ' . implode(' | ', $cells) . ' |';
}

/**
 * Точка входа: путь к данным — первый аргумент. / The entry point: the data path is the first argument.
 *
 * @param list<string> $arguments
 */
function main(array $arguments): void
{
    $data = $arguments[1] ?? 'data';
    $xml = (string) file_get_contents($data . '/catalog.xml');
    $xsd = (string) file_get_contents($data . '/catalog.xsd');
    $query = trim((string) file_get_contents($data . '/query.txt'));
    if (($arguments[2] ?? '') === '--memory') {
        // Режим памяти: одна библиотека в этом процессе / The memory mode: one library in this process
        echo Measure::memoryLine(engines($xsd, $query)[(int) $arguments[3]], $xml), PHP_EOL;
        return;
    }
    printf("### PHP %s, %d MB\n\n", PHP_VERSION, intdiv(strlen($xml), 1000000));
    echo '| Библиотека / Library | Разбор / Parse, MB/s | Обход / Walk, ms | XPath, ms | Разбор + XSD / Parse + XSD, MB/s ',
        '| Элементы / Elements | XPath: узлы / nodes | Ошибки XSD / XSD errors |', PHP_EOL;
    echo '|---|---|---|---|---|---|---|---|', PHP_EOL;
    foreach (engines($xsd, $query) as $engine) {
        echo row($engine, $xml), PHP_EOL;
    }
}

main($argv);
