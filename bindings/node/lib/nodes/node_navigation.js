'use strict';

const { native } = require('../internal/native.js');
const { wrap } = require('./wrapping.js');

/**
 * Примесь: навигация по дереву. Соседи и родитель находятся за O(1):
 * ядро хранит индекс узла у родителя. Отсутствующий сосед — null.
 *
 * Mixin: tree navigation. Siblings and the parent are found in O(1):
 * the core stores the node index in its parent. A missing sibling is null.
 */
class NodeNavigation
{
    /** Родитель (у корня — документ, у документа — null). / The parent (the document for the root, null for the document). */
    get parent()
    {
        return wrap(this.document, native.node_parent(this.handle));
    }

    /** Первый ребёнок или null. / The first child or null. */
    get firstChild()
    {
        return wrap(this.document, native.node_first_child(this.handle));
    }

    /** Последний ребёнок или null. / The last child or null. */
    get lastChild()
    {
        return wrap(this.document, native.node_last_child(this.handle));
    }

    /** Следующий сосед или null. / The next sibling or null. */
    get nextSibling()
    {
        return wrap(this.document, native.node_next_sibling(this.handle));
    }

    /** Предыдущий сосед или null. / The previous sibling or null. */
    get previousSibling()
    {
        return wrap(this.document, native.node_previous_sibling(this.handle));
    }
}

module.exports = { NodeNavigation };
