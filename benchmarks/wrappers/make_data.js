'use strict';

/*
 * Общие данные для замеров обёрток: каталог товаров, его схема и запрос
 * XPath пишутся в benchmarks/wrappers/data/, и программы замеров на Java,
 * C# и Python читают одни и те же файлы — значит, сравнивают одно и то же.
 *   node benchmarks/wrappers/make_data.js [число товаров]
 *
 * Common data for the wrapper measurements: a product catalog, its schema and an XPath
 * query are written to benchmarks/wrappers/data/, and the Java, C# and Python
 * benchmark programs read the same files — so they compare the same thing.
 *   node benchmarks/wrappers/make_data.js [number of products]
 */
const fs = require('node:fs');
const path = require('node:path');
const { makeCatalog, QUERY, CATALOG_XSD } = require('../node/workload.js');

// Точка входа: три файла в data/ / The entry point: three files in data/
function main()
{
    const items = Number(process.argv[2] || 50000);
    const directory = path.join(__dirname, 'data');
    const catalog = makeCatalog(items);

    fs.mkdirSync(directory, { recursive: true });
    fs.writeFileSync(path.join(directory, 'catalog.xml'), catalog);
    fs.writeFileSync(path.join(directory, 'catalog.xsd'), CATALOG_XSD);
    fs.writeFileSync(path.join(directory, 'query.txt'), QUERY);
    console.log('catalog.xml: ' + items + ' товаров / products, ' + (Buffer.byteLength(catalog) / 1e6).toFixed(1) + ' MB');
}

main();
