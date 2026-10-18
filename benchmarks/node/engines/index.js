'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { createRequire } = require('node:module');
const { berestaEngine } = require('./beresta.js');
const { createXmldomEngine } = require('./xmldom.js');
const { createFastXmlParserEngine } = require('./fast_xml_parser.js');
const { createLibxmljsEngine } = require('./libxmljs.js');

/*
 * Сторонние библиотеки ищутся в каталоге BENCH_NODE_MODULES (по умолчанию —
 * текущий): репозиторий от них не зависит, а для сравнения их ставят отдельно:
 *   npm install fast-xml-parser @xmldom/xmldom xpath libxmljs2
 * Ненайденная библиотека просто пропускается.
 *
 * Third-party libraries are looked up in the BENCH_NODE_MODULES directory (the
 * current one by default): the repository does not depend on them, and for the
 * comparison they are installed separately:
 *   npm install fast-xml-parser @xmldom/xmldom xpath libxmljs2
 * A library that is not found is simply skipped.
 */
const base = path.resolve(process.env.BENCH_NODE_MODULES || process.cwd());
const localRequire = createRequire(path.join(base, 'bench.js'));

// Библиотека по имени или null, если она не установлена / A library by name or null if it is not installed
function load(name)
{
    try
    {
        return localRequire(name);
    }
    catch (error)
    {
        return null;
    }
}

/*
 * Версия установленного пакета из его package.json. Файл читается напрямую:
 * поле exports пакета (как у fast-xml-parser) может закрывать его от require.
 *
 * The installed package version from its package.json. The file is read directly:
 * the package exports field (as in fast-xml-parser) may hide it from require.
 */
load.version = function version(name)
{
    try
    {
        const file = path.join(base, 'node_modules', name, 'package.json');
        return JSON.parse(fs.readFileSync(file, 'utf8')).version;
    }
    catch (error)
    {
        return '?';
    }
};

// Все доступные библиотеки; «Берёста» первая / All available libraries; Beresta is the first
function createEngines()
{
    const engines = [berestaEngine, createLibxmljsEngine(load), createXmldomEngine(load),
        createFastXmlParserEngine(load)];
    const result = [];

    for (const engine of engines)
    {
        if (engine !== null)
            result.push(engine);
    }
    return result;
}

module.exports = { createEngines };
