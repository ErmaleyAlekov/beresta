'use strict';

const { native } = require('../internal/native.js');
const { wrap } = require('../nodes/wrapping.js');

/*
 * Результат beresta_xpath_select читается внутри одного вызова: элементы
 * копируются в массив JavaScript, а нативный набор освобождается в finally —
 * даже если чтение прервало исключение.
 *
 * The result of beresta_xpath_select is read within one call: the items are
 * copied into a JavaScript array, and the native set is freed in finally —
 * even if reading was interrupted by an exception.
 */
function readNodeSet(xpath, context, readValues)
{
    const nodeset = native.xpath_select(xpath.handle, context.handle);
    const result = [];

    try
    {
        const size = native.nodeset_size(nodeset);
        for (let index = 0; index < size; ++index)
        {
            if (readValues)
                result.push(native.nodeset_value(nodeset, index));
            else
                appendNode(result, wrap(context.document, native.nodeset_node(nodeset, index)));
        }
    }
    finally
    {
        native.nodeset_free(nodeset);
    }
    return result;
}

// Узел попадает в результат, если это узел DOM (у атрибута узла нет)
// A node goes into the result if it is a DOM node (an attribute has no node)
function appendNode(result, node)
{
    if (node !== null)
        result.push(node);
}

/**
 * Примесь XPath: вычисление относительно узла или документа.
 * An XPath mixin: evaluation relative to a node or a document.
 */
class XPathEvaluation
{
    /** Узлы результата в порядке документа. Атрибуты в DOM «Берёсты» — не узлы;
     *  их значения возвращает selectValues.
     *  The result nodes in document order. Attributes in the Beresta DOM are not nodes;
     *  selectValues returns their values. */
    select(context)
    {
        return readNodeSet(this, context, false);
    }

    /** Строковые значения всех элементов результата, включая атрибуты.
     *  The string values of all result items, attributes included. */
    selectValues(context)
    {
        return readNodeSet(this, context, true);
    }
}

module.exports = { XPathEvaluation };
