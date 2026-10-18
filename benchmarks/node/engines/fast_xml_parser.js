'use strict';

/*
 * fast-xml-parser — самый популярный XML-парсер для Node.js. Строит не DOM, а
 * обычные объекты JavaScript; XPath и проверки по схеме нет. Атрибуты
 * включены (ignoreAttributes: false), чтобы работа была сопоставима с DOM.
 *
 * fast-xml-parser — the most popular XML parser for Node.js. It builds not a
 * DOM but plain JavaScript objects; there is no XPath and no schema validation.
 * Attributes are enabled (ignoreAttributes: false) so that the work is comparable to a DOM.
 */
function createFastXmlParserEngine(load)
{
    const library = load('fast-xml-parser');

    if (library === null)
        return null;
    const parser = new library.XMLParser({ ignoreAttributes: false, attributeNamePrefix: '@' });
    return {
        name: 'fast-xml-parser ' + load.version('fast-xml-parser'),
        parse(text)
        {
            return parser.parse(text);
        },
        walk(document)
        {
            return countValues(document);
        }
    };
}

// Число значений в дереве объектов (элементы, атрибуты, тексты) / The number of values in the object tree
function countValues(root)
{
    const stack = [root];
    let sum = 0;

    while (stack.length > 0)
    {
        const value = stack.pop();
        if (value === null || typeof value !== 'object')
        {
            sum += 1;
            continue;
        }
        for (const key of Object.keys(value))
            stack.push(value[key]);
        sum += 1;
    }
    return sum;
}

module.exports = { createFastXmlParserEngine };
