'use strict';

const path = require('node:path');

// Пакет из этого репозитория (собранный make NODE=1) / The package from this repository (built with make NODE=1)
const beresta = require(path.join(__dirname, '..', '..', '..', 'bindings', 'node', 'lib', 'index.js'));

/*
 * «Берёста» через обёртку Node-API. Обход идёт через firstChild / nextSibling —
 * так же, как у DOM-библиотек на JavaScript, — и каждый шаг пересекает
 * границу JavaScript и C++: это честная цена нативного DOM в Node.js.
 *
 * Beresta through the Node-API binding. The walk goes through firstChild /
 * nextSibling — like the JavaScript DOM libraries do — and each step crosses
 * the JavaScript / C++ boundary: this is the honest price of a native DOM in Node.js.
 */
const berestaEngine = {
    name: 'Берёста / Beresta ' + beresta.version() + ' (Node-API)',

    // Разбор строки в DOM / Parsing a string into DOM
    parse(text)
    {
        return beresta.parse(text);
    },

    // Число элементов, атрибутов и текстовых узлов / The number of elements, attributes and text nodes
    walk(document)
    {
        const stack = [document.root];
        let sum = 0;

        while (stack.length > 0)
        {
            const node = stack.pop();
            sum += 1;
            if (node.type === beresta.NodeType.ELEMENT)
                sum += Object.keys(node.attributes).length;
            for (let child = node.firstChild; child !== null; child = child.nextSibling)
                stack.push(child);
        }
        return sum;
    },

    // Число узлов результата XPath / The number of nodes in an XPath result
    xpath(document, query)
    {
        return new beresta.XPath(query).select(document).length;
    },

    // Освобождение нативной памяти / Freeing the native memory
    release(document)
    {
        document.close();
    }
};

module.exports = { berestaEngine };
