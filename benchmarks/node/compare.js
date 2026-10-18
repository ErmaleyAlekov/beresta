'use strict';

/*
 * Сравнение «Берёсты» с XML-библиотеками Node.js на одном каталоге товаров:
 * разбор строки, обход всего дерева и запрос XPath (у кого он есть).
 *   node benchmarks/node/compare.js [число товаров]
 * Сторонние библиотеки — необязательные (см. engines/index.js).
 *
 * A comparison of Beresta with Node.js XML libraries on one product catalog:
 * parsing a string, walking the whole tree and an XPath query (for those that
 * have it).
 *   node benchmarks/node/compare.js [number of products]
 * Third-party libraries are optional (see engines/index.js).
 */

const { makeCatalog, QUERY } = require('./workload.js');
const { createEngines } = require('./engines/index.js');

// Число повторов замера; берётся медиана / The number of measurement runs; the median is taken
const RUNS = 5;

// Операция дольше этого (мс) меряется один раз: повторы заняли бы минуты
// An operation longer than this (ms) is measured once: repetitions would take minutes
const SLOW_MS = 2000;

/*
 * Медиана времени (мс) нескольких запусков action; первый запуск — прогрев.
 * Очень медленная операция (дольше SLOW_MS) меряется одним запуском.
 *
 * The median time (ms) of several runs of action; the first run is a warm-up.
 * A very slow operation (longer than SLOW_MS) is measured with a single run.
 */
function medianTime(action, argument)
{
    const times = [];
    let result = null;

    for (let run = 0; run <= RUNS; ++run)
    {
        const start = process.hrtime.bigint();
        result = action(argument);
        const elapsed = Number(process.hrtime.bigint() - start) / 1e6;
        if (run === 0 && elapsed > SLOW_MS)
            return { ms: elapsed, result };
        if (run > 0)
            times.push(elapsed);
    }
    times.sort(compareNumbers);
    return { ms: times[Math.floor(times.length / 2)], result };
}

// Сравнение чисел для сортировки / Comparing numbers for sorting
function compareNumbers(left, right)
{
    return left - right;
}

/*
 * Все замеры одной библиотеки. Разбор меряется на строке JavaScript —
 * обычном входе для Node.js-кода; для «Берёсты» это включает перекодировку
 * в UTF-8 (Buffer.from).
 *
 * All measurements of one library. Parsing is measured on a JavaScript string —
 * the usual input for Node.js code; for Beresta this includes encoding into
 * UTF-8 (Buffer.from).
 */
function measureEngine(engine, text)
{
    const parsed = medianTime(engine.parse.bind(engine), text);
    const walked = medianTime(engine.walk.bind(engine), parsed.result);
    let xpath = '—';

    if (engine.xpath)
        xpath = medianTime(engine.xpath.bind(engine, parsed.result), QUERY).ms.toFixed(1);
    if (engine.release)
        engine.release(parsed.result);
    const megabytes = Buffer.byteLength(text) / 1e6;
    return '| ' + engine.name + ' | ' + (megabytes / parsed.ms * 1000).toFixed(0) + ' | '
        + walked.ms.toFixed(1) + ' | ' + xpath + ' | ' + walked.result + ' |';
}

// Точка входа: таблица Markdown в stdout / The entry point: a Markdown table to stdout
function main()
{
    const items = Number(process.argv[2] || 100000);
    const text = makeCatalog(items);

    console.log('### Node.js ' + process.version + ', catalog ' + (Buffer.byteLength(text) / 1e6).toFixed(0) + ' MB\n');
    console.log('| Библиотека / Library | Разбор / Parse, MB/s | Обход / Walk, ms | XPath, ms | Обход: узлы / Walk: nodes |');
    console.log('|---|---|---|---|---|');
    for (const engine of createEngines())
        console.log(measureEngine(engine, text));
}

main();
