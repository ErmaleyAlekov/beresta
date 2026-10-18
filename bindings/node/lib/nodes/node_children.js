'use strict';

const { native } = require('../internal/native.js');
const { wrap } = require('./wrapping.js');

/**
 * Примесь: доступ к детям — по индексу, массивом и перебором for...of.
 * Mixin: access to children — by index, as an array and with for...of.
 */
class NodeChildren
{
    /** Число детей (0 у листовых узлов). / The number of children (0 for leaf nodes). */
    get childCount()
    {
        return native.node_child_count(this.handle);
    }

    /** Ребёнок по индексу; отрицательный индекс — с конца, как у Array.prototype.at.
     *  A child by index; a negative index counts from the end, as in Array.prototype.at. */
    child(index)
    {
        let position = Math.trunc(index);

        if (position < 0)
            position += this.childCount;
        return wrap(this.document, native.node_child(this.handle, position));
    }

    /** Снимок списка детей: последующие изменения дерева в нём не видны.
     *  A snapshot of the child list: later tree changes are not visible in it. */
    get children()
    {
        return Array.from(this);
    }

    /** Перебор детей через соседей — O(1) на шаг. / Iterates children through siblings — O(1) per step. */
    *[Symbol.iterator]()
    {
        let current = this.firstChild;

        while (current !== null)
        {
            yield current;
            current = current.nextSibling;
        }
    }
}

module.exports = { NodeChildren };
