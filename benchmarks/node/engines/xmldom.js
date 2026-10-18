'use strict';

/*
 * @xmldom/xmldom — W3C DOM на чистом JavaScript (самая популярная DOM-
 * библиотека для Node.js). XPath — пакет xpath поверх того же DOM, если он
 * установлен. Обе библиотеки необязательны: без них строка просто не попадёт
 * в таблицу.
 *
 * @xmldom/xmldom — a W3C DOM in pure JavaScript (the most popular DOM library
 * for Node.js). XPath is the xpath package on top of the same DOM, if it is
 * installed. Both libraries are optional: without them the row simply does not
 * get into the table.
 */

// Разбор строки в DOM / Parsing a string into DOM
function parseWithXmldom(xmldom, text)
{
    return new xmldom.DOMParser().parseFromString(text, 'text/xml');
}

// Число элементов, атрибутов и текстовых узлов / The number of elements, attributes and text nodes
function walkDom(document)
{
    const stack = [document.documentElement];
    let sum = 0;

    while (stack.length > 0)
    {
        const node = stack.pop();
        sum += 1;
        if (node.nodeType === 1)
            sum += node.attributes.length;
        for (let child = node.firstChild; child !== null; child = child.nextSibling)
            stack.push(child);
    }
    return sum;
}

// XPath через пакет xpath / XPath through the xpath package
function selectWithXpath(xpath, document, query)
{
    return xpath.select(query, document).length;
}

// Адаптер или null, если @xmldom/xmldom не установлен / The adapter or null if @xmldom/xmldom is not installed
function createXmldomEngine(load)
{
    const xmldom = load('@xmldom/xmldom');
    const xpath = load('xpath');

    if (xmldom === null)
        return null;
    const engine = {
        name: '@xmldom/xmldom ' + load.version('@xmldom/xmldom'),
        parse: parseWithXmldom.bind(null, xmldom),
        walk: walkDom
    };
    if (xpath !== null)
        engine.xpath = selectWithXpath.bind(null, xpath);
    return engine;
}

module.exports = { createXmldomEngine };
