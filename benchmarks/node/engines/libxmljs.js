'use strict';

/*
 * libxmljs2 — привязка libxml2 к Node.js через собственный C++-аддон
 * (собирается node-gyp при установке). Полный DOM, XPath и XSD — ближайший
 * по возможностям аналог «Берёсты» в мире Node.js.
 *
 * libxmljs2 — a libxml2 binding for Node.js through its own C++ addon
 * (built by node-gyp at installation). A full DOM, XPath and XSD — the closest
 * analog of Beresta by features in the Node.js world.
 */

// Число элементов, атрибутов и текстовых узлов / The number of elements, attributes and text nodes
function walkLibxmljs(document)
{
    const stack = [document.root()];
    let sum = 0;

    while (stack.length > 0)
    {
        const node = stack.pop();
        sum += 1;
        if (node.type() !== 'element')
            continue;
        sum += node.attrs().length;
        for (const child of node.childNodes())
            stack.push(child);
    }
    return sum;
}

// Число узлов результата XPath / The number of nodes in an XPath result
function findLibxmljs(document, query)
{
    return document.find(query).length;
}

// Адаптер или null, если libxmljs2 не установлена / The adapter or null if libxmljs2 is not installed
function createLibxmljsEngine(load)
{
    const libxmljs = load('libxmljs2');

    if (libxmljs === null)
        return null;
    return {
        name: 'libxmljs2 ' + load.version('libxmljs2'),
        parse: libxmljs.parseXml.bind(libxmljs),
        walk: walkLibxmljs,
        xpath: findLibxmljs
    };
}

module.exports = { createLibxmljsEngine };
